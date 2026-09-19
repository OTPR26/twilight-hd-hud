"""Verify the distribution bundle, not merely individual build artifacts."""
import json
import struct
import sys
import zipfile
from windows_package_test import verify

expected = {
    'windows-amd64': 'mod.dll', 'windows-arm64': 'mod.dll',
    'linux-x86_64': 'mod.so', 'linux-aarch64': 'mod.so',
    'macos-arm64': 'mod.so', 'macos-x86_64': 'mod.so',
    'android-aarch64': 'mod.so', 'ios-arm64': 'mod.so', 'tvos-arm64': 'mod.so',
}
path = sys.argv[1]
with zipfile.ZipFile(path) as package:
    assert package.testzip() is None
    names = package.namelist()
    assert len(names) == len(set(names))
    libraries = {n for n in names if n.startswith('lib/') and not n.endswith('/')}
    assert libraries == {f'lib/{p}/{n}' for p, n in expected.items()}, libraries
    for platform, filename in expected.items():
        data = package.read(f'lib/{platform}/{filename}')
        if platform.startswith(('linux', 'android')):
            assert data[:6] == b'\x7fELF\x02\x01'
            assert struct.unpack_from('<H', data, 18)[0] == (62 if platform.endswith('x86_64') else 183)
        elif platform.startswith(('macos', 'ios', 'tvos')):
            assert data[:4] == b'\xcf\xfa\xed\xfe'
            assert struct.unpack_from('<I', data, 4)[0] == (0x1000007 if platform.endswith('x86_64') else 0x100000c)
    manifest = json.loads(package.read('mod.json'))
    assert manifest['abi'] == 1 and manifest['imports'] and manifest['exports'] == []
    for entry in package.infolist():
        assert entry.file_size <= 200 * max(1, entry.compress_size), entry.filename
        assert not entry.filename.startswith('/') and '..' not in entry.filename.split('/')
        if entry.filename.startswith('textures/') and entry.filename.endswith('.dds'):
            assert struct.unpack_from('<I', package.read(entry), 128)[0] == 28
verify(path, 'amd64')
verify(path, 'arm64')
print('PASS: all eight required targets plus tvOS, metadata, textures, CRCs, and compression limits')
