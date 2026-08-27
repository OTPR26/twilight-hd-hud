#include "font_override.hpp"
#include "font_atlas.hpp"
#include "config.hpp"
#include "service_imports.hpp"

#include "JSystem/JKernel/JKRHeap.h"
#include "m_Do/m_Do_ext.h"
#include "mods/svc/hook.hpp"
#include <cstdio>

namespace twilight_hd_hud {
namespace {

DEFINE_HOOK(&mDoExt_getMesgFont, MessageFontAcquireHook);
DEFINE_HOOK(&JUTResFont::deleteMemBlocks_ResFont, FontReleaseHook);

TextFont s_activeFont = TextFont::Original;
ResourceBuffer s_resource = RESOURCE_BUFFER_INIT;
JUTFont* s_messageFont = nullptr;  // Observed, never retained or freed by this mod.
JUTResFont* s_replacement = nullptr;
bool s_attemptedConstruction = false;
bool s_loggedDraw = false;

void after_acquire_message_font(ModContext*, void*, void* retval, void*) {
    if (s_activeFont != TextFont::Original && retval) {
        s_messageFont = *static_cast<JUTFont**>(retval);
    }
}

HookAction before_release_font(ModContext*, void* args, void*, void*) {
    if (mods::arg<JUTResFont*>(args, 0) == s_messageFont) {
        s_messageFont = nullptr;
    }
    return HOOK_CONTINUE;
}

bool ensure_replacement() {
    if (s_replacement) return true;
    if (s_attemptedConstruction || !s_resource.data) return false;
    s_attemptedConstruction = true;
    // Font creation touches GX and game heaps: defer until a real game font is drawing.
    // Use the persistent root heap, never a transient menu or stage heap.
    auto* heap = JKRHeap::getRootHeap();
    if (!heap) return false;
    s_replacement = JKR_NEW_ARGS(heap, 32) JUTResFont(
        static_cast<const ResFONT*>(s_resource.data), heap);
    if (!s_replacement || !s_replacement->isValid()) {
        JKR_DELETE(s_replacement);
        s_replacement = nullptr;
        svc_log->warn(mod_ctx, "Text Font: native font construction failed; using Original");
        return false;
    }
    return true;
}

}  // namespace

void initialize_font_override() {
    // Latch once: changing the saved selection must not swap a live GPU font.
    s_activeFont = text_font();
    if (s_activeFont == TextFont::Original) return;
    const char* path = nullptr;
    const char* name = nullptr;
    switch (s_activeFont) {
    case TextFont::ZenKakuGothicNew:
        path = "fonts/zen-bold.bfn";
        name = "Zen Kaku Gothic New";
        break;
    case TextFont::MPlus2:
        path = "fonts/mplus-bold.bfn";
        name = "M PLUS 2";
        break;
    case TextFont::FiraSans:
        path = "fonts/fira-bold.bfn";
        name = "Dusklight - Fira Sans Bold";
        break;
    default:
        s_activeFont = TextFont::Original;
        return;
    }
    if (svc_resource->load(mod_ctx, path, &s_resource) != MOD_OK ||
        !font_atlas::valid(s_resource.data, s_resource.size)) {
        svc_log->warn(mod_ctx, "Text Font: missing or invalid atlas; using Original");
        svc_resource->free(mod_ctx, &s_resource);
        s_activeFont = TextFont::Original;
        return;
    }
    if (mods::hook::add_post<MessageFontAcquireHook>(svc_hook, after_acquire_message_font) != MOD_OK ||
        mods::hook::add_pre<FontReleaseHook>(svc_hook, before_release_font) != MOD_OK) {
        // Any successfully installed observer remains inert and is owned by the loader.
        svc_log->warn(mod_ctx, "Text Font: font lifecycle hooks unavailable; using Original");
        svc_resource->free(mod_ctx, &s_resource);
        s_activeFont = TextFont::Original;
        return;
    }
    char selection[160];
    std::snprintf(selection, sizeof(selection), "Text Font: %s selected", name);
    svc_log->info(mod_ctx, selection);
}

void shutdown_font_override() {
    s_activeFont = TextFont::Original;
    s_messageFont = nullptr;
    // Game panes still point at their original fonts. Only our draw-only object is freed.
    JKR_DELETE(s_replacement);
    s_replacement = nullptr;
    svc_resource->free(mod_ctx, &s_resource);
    s_attemptedConstruction = false;
    s_loggedDraw = false;
}

bool draw_font_override(void* args, void* retval, FontDrawOriginal drawOriginal) {
    if (s_activeFont == TextFont::Original || !retval || !drawOriginal) return false;
    auto* source = mods::arg<JUTResFont*>(args, 0);
    const int code = mods::arg<int>(args, 5);
    // Only the game's message font is replaced; ruby, decorative and debug fonts are untouched.
    if (!source || source != s_messageFont || source->getFontType() != 0 ||
        !font_atlas::supported(code) || source->getCellWidth() <= 0) return false;
    const float scaleX = mods::arg<f32>(args, 3);
    const float scaleY = mods::arg<f32>(args, 4);
    // Preserve unusual mirrored/hidden draw paths rather than inventing their geometry.
    if (scaleX <= 0 || scaleY <= 0 || !ensure_replacement()) return false;

    const bool subsequent = mods::arg<bool>(args, 6);
    JUTFont::TWidth nativeWidth{};
    source->getWidthEntry(code, &nativeWidth);
    JUTFont::TWidth replacementWidth{};
    s_replacement->getWidthEntry(code, &replacementWidth);
    const float rasterScale = s_activeFont == TextFont::FiraSans ?
        font_atlas::firaOpticalScale : font_atlas::opticalScale;
    const auto placement = font_atlas::place(mods::arg<f32>(args, 1), scaleX,
        source->getCellWidth(), nativeWidth.field_0x0, nativeWidth.field_0x1,
        replacementWidth.field_0x1, source->mFixed, source->mFixedWidth, subsequent, rasterScale);
    const float nativeAdvance = font_atlas::advance(source->mFixed, source->mFixedWidth,
        subsequent, nativeWidth.field_0x0, nativeWidth.field_0x1, scaleX,
        source->getCellWidth());

    s_replacement->mColor1 = source->mColor1;
    s_replacement->mColor2 = source->mColor2;
    s_replacement->mColor3 = source->mColor3;
    s_replacement->mColor4 = source->mColor4;
    s_replacement->mFixed = false;
    auto* context = mods::arg<FontDrawContext*>(args, 7);
    // The context belongs to the original font. Never leave its cache claiming that its
    // texture is loaded when ours is bound (especially at a fallback glyph boundary).
    if (context) context->isTextureLoaded = false;
    drawOriginal(s_replacement, placement.x, mods::arg<f32>(args, 2),
        placement.scaleX, scaleY * rasterScale, code, true, context);
    if (context) context->isTextureLoaded = false;
    *static_cast<f32*>(retval) = nativeAdvance;
    if (!s_loggedDraw) {
        svc_log->info(mod_ctx, "Text Font: replacement drawing active; original advances preserved");
        char metrics[160];
        std::snprintf(metrics, sizeof(metrics), "Text Font: original cell=%dx%d ascent=%d descent=%d scale=%.2fx%.2f",
            source->getCellWidth(), source->getCellHeight(), source->getAscent(), source->getDescent(), scaleX, scaleY);
        svc_log->info(mod_ctx, metrics);
        s_loggedDraw = true;
    }
    return true;
}

}  // namespace twilight_hd_hud
