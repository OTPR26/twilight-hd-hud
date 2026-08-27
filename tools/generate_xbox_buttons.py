"""Generate Xbox labels using the mod's existing shoulder-cap artwork system."""
from PIL import Image
from generate_playstation_buttons import ROOT, OUTPUT_DIR, encode_rgba8_bti, make_shoulder_button


def main():
    source_dir = ROOT / "assets" / "source" / "xbox"
    source_dir.mkdir(parents=True, exist_ok=True)
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    preview = Image.new("RGBA", (4 * 128, 2 * 128), (35, 35, 35, 255))
    for row, (black, suffix) in enumerate(((False, ""), (True, "-black-pro"))):
        for col, label in enumerate(("LB", "RB", "LT", "RT")):
            name = f"shoulder-button-xbox-{label.lower()}{suffix}"
            image = make_shoulder_button(label, black)
            image.save(source_dir / f"{name}.png")
            (OUTPUT_DIR / f"{name}.bti").write_bytes(encode_rgba8_bti(image))
            preview.alpha_composite(image.resize((128, 128)), (col * 128, row * 128))
    preview.save(source_dir / "preview.png")


if __name__ == "__main__":
    main()
