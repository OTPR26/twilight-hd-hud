from pathlib import Path

source = (Path(__file__).parents[1] / "src" / "update_service.cpp").read_text()

replacement = source.split("bool download_and_replace(", 1)[1].split(
    "void show_dialog(", 1
)[0]
assert 'temporary = target.string() + ".download"' in replacement
assert 'backup = target.string() + ".previous"' in replacement
assert "fs::rename(target, backup, ec)" in replacement
assert "fs::rename(temporary, target, ec)" in replacement
success = replacement.split("fs::rename(temporary, target, ec)", 1)[1].split(
    "fs::remove(temporary, ec)", 1
)[0]
assert "fs::remove(backup, ec)" in success

initialization = source.split("void initialize_update_service()", 1)[1].split(
    "void update_update_service()", 1
)[0]
assert "remove_update_backup();" in initialization

print("PASS: updater backup is transactional and stale v1.8.0 backup is removed")
