"""Check that a Windows package contains the requested native architecture."""
import argparse
import struct
import zipfile


def verify(path, architecture):
    machines = {"amd64": 0x8664, "arm64": 0xAA64}
    member = f"lib/windows-{architecture}/mod.dll"
    with zipfile.ZipFile(path) as package:
        assert package.testzip() is None, "Archive checksum failure"
        assert package.namelist().count(member) == 1, f"Missing or duplicate {member}"
        dll = package.read(member)
    assert dll[:2] == b"MZ", "Not a PE executable"
    pe_offset = struct.unpack_from("<I", dll, 0x3C)[0]
    assert dll[pe_offset:pe_offset + 4] == b"PE\0\0", "Missing PE signature"
    machine = struct.unpack_from("<H", dll, pe_offset + 4)[0]
    assert machine == machines[architecture], f"Unexpected PE machine: {machine:#x}"
    print(f"Verified {member}: PE machine {machine:#x}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("package")
    parser.add_argument("architecture", choices=("amd64", "arm64"))
    args = parser.parse_args()
    verify(args.package, args.architecture)
