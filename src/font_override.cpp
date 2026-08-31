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
ResourceBuffer s_itemPromptResource = RESOURCE_BUFFER_INIT;
JUTResFont* s_itemPromptFont = nullptr;
bool s_attemptedItemPromptConstruction = false;
int s_itemPromptDepth = 0;
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

bool ensure_item_prompt_font() {
    if (s_itemPromptFont) return true;
    if (s_attemptedItemPromptConstruction || !s_itemPromptResource.data) return false;
    s_attemptedItemPromptConstruction = true;
    auto* heap = JKRHeap::getRootHeap();
    if (!heap) return false;
    s_itemPromptFont = JKR_NEW_ARGS(heap, 32) JUTResFont(
        static_cast<const ResFONT*>(s_itemPromptResource.data), heap);
    if (!s_itemPromptFont || !s_itemPromptFont->isValid()) {
        JKR_DELETE(s_itemPromptFont);
        s_itemPromptFont = nullptr;
        svc_log->warn(mod_ctx,
            "Item prompt font: native Fira Sans construction failed; using selected font");
        return false;
    }
    return true;
}

}  // namespace

void initialize_font_override() {
    // Item-acquisition cards deliberately use the same clean, high-legibility
    // Fira presentation on every platform.  Keep this resource independent of
    // the user's global dialogue-font choice so ordinary dialogue is untouched.
    if (svc_resource->load(mod_ctx, "fonts/fira-bold.bfn", &s_itemPromptResource) != MOD_OK ||
        !font_atlas::valid(s_itemPromptResource.data, s_itemPromptResource.size)) {
        svc_log->warn(mod_ctx,
            "Item prompt font: missing or invalid Fira Sans atlas; using selected font");
        svc_resource->free(mod_ctx, &s_itemPromptResource);
    }
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
    JKR_DELETE(s_itemPromptFont);
    s_itemPromptFont = nullptr;
    svc_resource->free(mod_ctx, &s_resource);
    svc_resource->free(mod_ctx, &s_itemPromptResource);
    s_attemptedConstruction = false;
    s_attemptedItemPromptConstruction = false;
    s_itemPromptDepth = 0;
    s_loggedDraw = false;
}

void begin_item_prompt_font() {
    ++s_itemPromptDepth;
}

void end_item_prompt_font() {
    if (s_itemPromptDepth > 0) --s_itemPromptDepth;
}

bool draw_font_override(void* args, void* retval, FontDrawOriginal drawOriginal) {
    if (!retval || !drawOriginal) return false;
    auto* source = mods::arg<JUTResFont*>(args, 0);
    const int code = mods::arg<int>(args, 5);
    // The item-card draw scope has a dedicated Fira face. Outside that narrow
    // scope, preserve the selected global message-font behavior exactly.
    const bool itemPrompt = s_itemPromptDepth > 0 && ensure_item_prompt_font();
    if (!source || source->getFontType() != 0 || !font_atlas::supported(code) ||
        source->getCellWidth() <= 0 ||
        (!itemPrompt && (s_activeFont == TextFont::Original || source != s_messageFont))) {
        return false;
    }
    const float scaleX = mods::arg<f32>(args, 3);
    const float scaleY = mods::arg<f32>(args, 4);
    // Preserve unusual mirrored/hidden draw paths rather than inventing their geometry.
    if (scaleX <= 0 || scaleY <= 0 || (!itemPrompt && !ensure_replacement())) return false;

    JUTResFont* replacement = itemPrompt ? s_itemPromptFont : s_replacement;

    const bool subsequent = mods::arg<bool>(args, 6);
    JUTFont::TWidth nativeWidth{};
    source->getWidthEntry(code, &nativeWidth);
    JUTFont::TWidth replacementWidth{};
    replacement->getWidthEntry(code, &replacementWidth);
    const float rasterScale = itemPrompt || s_activeFont == TextFont::FiraSans ?
        font_atlas::firaOpticalScale : font_atlas::opticalScale;
    const bool itemStem = itemPrompt &&
        (code == 'i' || code == 'j' || code == 'l' || code == 'I');
    const auto placement = font_atlas::place(mods::arg<f32>(args, 1), scaleX,
        source->getCellWidth(), nativeWidth.field_0x0, nativeWidth.field_0x1,
        replacementWidth.field_0x1, source->mFixed, source->mFixedWidth, subsequent,
        rasterScale, itemStem ? 0.5f : 0.0f);
    const float nativeAdvance = font_atlas::advance(source->mFixed, source->mFixedWidth,
        subsequent, nativeWidth.field_0x0, nativeWidth.field_0x1, scaleX,
        source->getCellWidth());
    const float renderedAdvance = replacementWidth.field_0x1 * placement.scaleX /
        font_atlas::cell;
    const float drawAdvance = itemStem ?
        std::max(nativeAdvance, renderedAdvance) : nativeAdvance;

    replacement->mColor1 = source->mColor1;
    replacement->mColor2 = source->mColor2;
    replacement->mColor3 = source->mColor3;
    replacement->mColor4 = source->mColor4;
    replacement->mFixed = false;
    auto* context = mods::arg<FontDrawContext*>(args, 7);
    // The context belongs to the original font. Never leave its cache claiming that its
    // texture is loaded when ours is bound (especially at a fallback glyph boundary).
    if (context) context->isTextureLoaded = false;
    drawOriginal(replacement, placement.x, mods::arg<f32>(args, 2),
        placement.scaleX, scaleY * rasterScale, code, true, context);
    if (context) context->isTextureLoaded = false;
    *static_cast<f32*>(retval) = drawAdvance;
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
