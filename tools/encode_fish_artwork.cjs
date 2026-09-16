// Compile transparent source artwork without changing its painted pixels.
// Transparent margins are trimmed; aspect ratio and generated alpha retained.
const fs = require('node:fs/promises');
const path = require('node:path');
const sharp = require('sharp');
const { encodeRGBA8 } = require('./encode_collection_vectors.cjs');
const root = path.resolve(__dirname, '..');
const names = ['hyrule-bass', 'hylian-loach', 'hylian-pike',
    'ordon-catfish', 'reekfish', 'greengill'];

(async () => {
    const output = path.join(root, 'res/menu/fish');
    await fs.mkdir(output, { recursive: true });
    for (const name of names) {
        const input = path.join(root, 'assets/source/fish', name + '.png');
        const { data, info } = await sharp(input).ensureAlpha().raw().toBuffer({ resolveWithObject: true });
        let left = info.width, top = info.height, right = -1, bottom = -1;
        let transparent = 0;
        for (let y = 0; y < info.height; ++y) for (let x = 0; x < info.width; ++x) {
            if (data[(y * info.width + x) * 4 + 3] === 0) { ++transparent; continue; }
            left = Math.min(left, x); top = Math.min(top, y);
            right = Math.max(right, x); bottom = Math.max(bottom, y);
        }
        if (transparent < info.width * info.height * 0.2 || right < left)
            throw new Error(name + ': missing genuine transparent background');
        const crop = { left, top, width: right - left + 1, height: bottom - top + 1 };
        const height = Math.round(crop.height * 504 / crop.width);
        const textureHeight = Math.ceil((height + 8) / 4) * 4;
        const { data: pixels, info: dimensions } = await sharp(input).extract(crop)
            .resize(504, height).extend({ left: 4, right: 4, top: 4,
                bottom: textureHeight - height - 4, background: { r: 0, g: 0, b: 0, alpha: 0 } })
            .ensureAlpha().raw().toBuffer({ resolveWithObject: true });
        await fs.writeFile(path.join(output, name + '.bti'),
            encodeRGBA8(pixels, dimensions.width, dimensions.height));
        console.log(name, dimensions.width, dimensions.height, 'RGBA8, alpha preserved');
    }
})().catch(error => { console.error(error); process.exit(1); });
