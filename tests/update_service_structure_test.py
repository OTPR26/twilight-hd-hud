from pathlib import Path

source = (Path(__file__).parents[1] / "src" / "update_service.cpp").read_text()
mod_source = (Path(__file__).parents[1] / "src" / "mod.cpp").read_text()
imports = (Path(__file__).parents[1] / "src" / "service_imports.hpp").read_text()
cmake = (Path(__file__).parents[1] / "CMakeLists.txt").read_text()

replacement = source.split("bool replace_download(", 1)[1].split(
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

assert '#include "mods/svc/http.h"' in source
assert "IMPORT_SERVICE(HttpService, svc_http);" in mod_source
assert "extern const HttpService* svc_http;" in imports
assert "svc_http->request(mod_ctx, &request, on_check_complete" in source
assert "svc_http->request(mod_ctx, &request, on_download_complete" in source
assert "request.download_path = stagingText.c_str();" in source
assert "svc_http->cancel(mod_ctx, s_request);" in source
assert "InternetOpen" not in source
assert "popen(" not in source
assert "std::system(" not in source
assert "(void)url;" not in source
assert "wininet" not in cmake.lower()

initialization = source.split("void initialize_update_service()", 1)[1].split(
    "void update_update_service()", 1
)[0]
assert "remove_update_backup();" in initialization

print("PASS: updater uses the cross-platform HTTP service and transactional replacement")
