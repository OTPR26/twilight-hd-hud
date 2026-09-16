// Fit generated transparent artwork to the existing silver face-button canvas.
const fs = require('node:fs/promises');
const path = require('node:path');
const sharp = require('sharp');
const { encodeRGBA8 } = require('./encode_collection_vectors.cjs');
const root = path.resolve(__dirname, '..');
function bounds(data, width, height) {
    let left = width, top = height, right = -1, bottom = -1;
    for (let y = 0; y < height; ++y) for (let x = 0; x < width; ++x) {
        if (data[(y * width + x) * 4 + 3] <= 10) continue;
        left = Math.min(left, x); top = Math.min(top, y);
        right = Math.max(right, x); bottom = Math.max(bottom, y);
    }
    if (right < left) throw new Error('Empty sprite');
    return { left, top, width: right - left + 1, height: bottom - top + 1 };
}
(async () => {
    const reference = await fs.readFile(path.join(root, 'res/hud/face-button-a.bti'));
    if (reference[0] !== 3) throw new Error('Expected IA8 reference');
    const width = reference.readUInt16BE(2), height = reference.readUInt16BE(4);
    const original = Buffer.alloc(width * height * 4);
    let offset = reference.readUInt32BE(28);
    for (let y = 0; y < height; y += 4) for (let x = 0; x < width; x += 4) {
        for (let i = 0; i < 16; ++i)
            original[((y + (i >> 2)) * width + x + (i & 3)) * 4 + 3] = reference[offset + i * 2];
        offset += 32;
    }
    const target = bounds(original, width, height);
    const input = path.join(root, 'assets/source/buttons/face-button-blank-silver.png');
    const { data, info } = await sharp(input).ensureAlpha().raw().toBuffer({ resolveWithObject: true });
    if (!data.some((v, i) => i % 4 === 3 && v === 0)) throw new Error('Missing transparency');
    const crop = bounds(data, info.width, info.height);
    const pixels = await sharp(input).extract(crop).resize(target.width, target.height, { fit: 'fill' })
        .extend({ left: target.left, top: target.top,
            right: width - target.left - target.width, bottom: height - target.top - target.height,
            background: { r: 0, g: 0, b: 0, alpha: 0 } })
        .ensureAlpha().raw().toBuffer();
    await fs.writeFile(path.join(root, 'res/hud/face-button-blank-silver.bti'), encodeRGBA8(pixels, width, height));
    console.log('Universal Silver:', width, height, 'RGBA8 with transparent margins');
})().catch(error => { console.error(error); process.exitCode = 1; });
