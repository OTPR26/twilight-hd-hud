// Compile the editable menu SVG decorations into tiled GameCube RGBA8.
// Requires sharp; no game assets are copied into these generated resources.
const fs = require('node:fs/promises');
const path = require('node:path');
const sharp = require('sharp');
const root = path.resolve(__dirname, '..');

function encodeRGBA8(pixels, width, height) {
    if (width % 4 || height % 4) throw new Error('RGBA8 dimensions must be multiples of four');
    const result = Buffer.alloc(32 + width * height * 4);
    result[0] = 6;
    result[1] = 1;
    result.writeUInt16BE(width, 2);
    result.writeUInt16BE(height, 4);
    result[20] = result[21] = result[24] = 1;
    result.writeUInt32BE(32, 28);
    let offset = 32;
    for (let y = 0; y < height; y += 4) {
        for (let x = 0; x < width; x += 4) {
            for (let i = 0; i < 16; ++i) {
                const p = ((y + (i >> 2)) * width + x + (i & 3)) * 4;
                result[offset + i * 2] = pixels[p + 3];
                result[offset + i * 2 + 1] = pixels[p];
                result[offset + 32 + i * 2] = pixels[p + 1];
                result[offset + 33 + i * 2] = pixels[p + 2];
            }
            offset += 64;
        }
    }
    return result;
}

(async () => {
    const names = ['collection-banner', 'collection-equipment-frame', 'item-bank-cell',
        'item-bank-circle', 'item-bank-shadow', 'dungeon-map-frame', 'dungeon-map-back-dpad', 'overworld-map-frame'];
    for (const name of process.argv[3] ? names.filter(name => name === process.argv[3]) : names) {
        const source = path.join(root, 'assets/source', name + '.svg');
        const {data, info} = await sharp(source).ensureAlpha().raw().toBuffer({resolveWithObject: true});
        await fs.writeFile(path.join(root, 'res/menu', name + '.bti'), encodeRGBA8(data, info.width, info.height));
        if (process.argv[2]) {
            await fs.mkdir(process.argv[2], {recursive: true});
            await sharp(source).png().toFile(path.join(process.argv[2], name + '.png'));
        }
    }
})().catch(error => { console.error(error); process.exitCode = 1; });
