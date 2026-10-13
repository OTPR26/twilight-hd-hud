#include "../src/update_service.cpp"
#include <cassert>
#include <chrono>
#include <sstream>

ModContext* mod_ctx = nullptr;
const ConfigService* svc_config = nullptr;
const HostService* svc_host = nullptr;
const HttpService* svc_http = nullptr;
const UiService* svc_ui = nullptr;

namespace twilight_hd_hud {
bool check_for_updates_enabled() { return true; }
ConfigVarHandle check_for_updates_config_var() { return 1; }
}
using namespace twilight_hd_hud;
namespace fs = std::filesystem;
static HttpCompleteFn completion = nullptr;
static std::string requestedUrl, downloadPath, dataPath, scratchPath, dialogTitle, dialogBody;
static UiDialogActionFn installAction = nullptr, dismissAction = nullptr;
static HttpRequestHandle nextHandle = 1;
static uint64_t bodyLimit = 0;
static ModResult request(ModContext*, const HttpRequestDesc* desc, HttpCompleteFn fn, void*, HttpRequestHandle* handle) {
    completion = fn; requestedUrl = desc->url;
    downloadPath = desc->download_path ? desc->download_path : "";
    bodyLimit = desc->max_body_bytes;
    *handle = nextHandle++; return MOD_OK;
}
static ModResult cancel(ModContext*, HttpRequestHandle) { return MOD_OK; }
static ModResult data_dir(ModContext*, const char** path) { *path=dataPath.c_str(); return MOD_OK; }
static const char* mod_dir(ModContext*) { return scratchPath.c_str(); }
static const char* version(ModContext*) { return "2.4.4"; }
static ModResult subscribe(ModContext*, ConfigVarHandle, ConfigChangedFn, void*, uint64_t*) { return MOD_OK; }
static ModResult dialog(ModContext*, const UiDialogDesc* desc, UiDialogHandle*) {
    dialogBody=desc->body_rml; dialogTitle=desc->title; dismissAction=desc->on_dismiss;
    installAction=desc->action_count==2 ? desc->actions[0].on_pressed : nullptr;
    return MOD_OK;
}
static void complete_check(const std::string& json) {
    HttpResult response{}; response.error=HTTP_ERROR_NONE; response.status_code=200;
    response.body=json.data(); response.body_size=json.size();
    completion(nullptr, 1, &response, nullptr); update_update_service();
}
static void write(const fs::path& path, const std::string& bytes) {
    std::ofstream file(path,std::ios::binary); file.write(bytes.data(),bytes.size()); assert(file.good());
}
static std::string read(const fs::path& path) {
    std::ifstream file(path,std::ios::binary); return {std::istreambuf_iterator<char>(file),{}};
}
int main() {
    const fs::path temp=fs::temp_directory_path()/std::string("hud-update-test-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    dataPath=(temp/"mod_data/org.twilight.hd_hud").string(); fs::create_directories(dataPath); fs::create_directories(temp/"mods");
    ConfigService config{}; config.subscribe=subscribe; svc_config=&config;
    HostService host{}; host.mod_version=version; host.data_dir=data_dir; host.mod_dir=mod_dir; svc_host=&host;
    HttpService http{}; http.request=request; http.cancel=cancel; svc_http=&http;
    UiService ui{}; ui.dialog_push=dialog; svc_ui=&ui;
    const auto package=temp/"mods"/kReleaseAsset;
    write(package,"existing package");
    std::string payload="PK\3\4"+std::string(2044,'x');
    std::istringstream stream(payload); const auto digest=update_policy::sha256(stream);
    const std::string json=std::string("{\"tag_name\":\"v2.5.0\",\"assets\":[{\"name\":\"")+kReleaseAsset+
        "\",\"size\":2048,\"digest\":\"sha256:"+digest+"\",\"browser_download_url\":\"https://github.com/OTPR26/twilight-hd-hud/releases/download/v2.5.0/"+kReleaseAsset+"\"}]}";
    initialize_update_service();
    s_executablePath = temp/"installation/dusklight.exe";
    assert(requestedUrl.find("api.github.com/repos/OTPR26/twilight-hd-hud/")!=std::string::npos);
    complete_check(json);
    if (kCanSelfInstall) {
        assert(installAction && update_service_busy(nullptr,nullptr));
        request_update_check(nullptr,reinterpret_cast<void*>(1)); // Cannot replace a pending offer.
        assert(s_state.load()==State::AwaitingChoice);
        dismissAction(nullptr,0,nullptr); assert(!update_service_busy(nullptr,nullptr));
        request_update_check(nullptr,reinterpret_cast<void*>(1)); complete_check(json);
        installAction(nullptr,0,nullptr); assert(!downloadPath.empty());
        write(downloadPath,payload.substr(0,2047)+"y");
        HttpResult downloaded{}; downloaded.error=HTTP_ERROR_NONE; downloaded.status_code=200;
        downloaded.download_path=downloadPath.c_str();
        completion(nullptr,2,&downloaded,nullptr); update_update_service();
        assert(dialogTitle=="Update Failed" && read(package)=="existing package");
        assert(!dialogBody.empty());
        assert(!fs::exists(downloadPath));
        // Completion failures must retain the host's reason instead of blaming
        // the connection for disk errors, timeouts, or unsuccessful HTTP status.
        for (const auto error : {HTTP_ERROR_IO, HTTP_ERROR_TIMEOUT, HTTP_ERROR_NETWORK}) {
            request_update_check(nullptr,reinterpret_cast<void*>(1)); complete_check(json);
            installAction(nullptr,0,nullptr); write(downloadPath,payload);
            HttpResult failed{}; failed.error=error; failed.status_code=200;
            failed.error_message="Native failure <details> & code 17";
            completion(nullptr,2,&failed,nullptr); update_update_service();
            assert(dialogTitle=="Update Failed" && read(package)=="existing package");
            assert(dialogBody.find("Native failure &lt;details&gt; &amp; code 17")!=std::string::npos);
            if (error==HTTP_ERROR_IO) assert(dialogBody.find("could not save")!=std::string::npos);
            if (error==HTTP_ERROR_TIMEOUT) assert(dialogBody.find("timed out")!=std::string::npos);
            assert(!fs::exists(downloadPath));
        }
        HttpResult rejected{}; rejected.error=HTTP_ERROR_NONE; rejected.status_code=403;
        assert(download_error(&rejected)=="GitHub returned HTTP 403.");
        assert(download_error(nullptr)=="Dusklight returned no download result.");
        // Windows bypasses the host's file publisher and receives a bounded body.
        for (int scenario=0; scenario<4; ++scenario) {
            request_update_check(nullptr,reinterpret_cast<void*>(1)); complete_check(json);
            confirm_update_with_transport(true);
            assert(downloadPath.empty() && bodyLimit==payload.size());
            std::string body=payload;
            HttpResult buffered{}; buffered.error=HTTP_ERROR_NONE; buffered.status_code=200;
            if (scenario==0) body.pop_back(); // Incomplete response cannot replace the package.
            if (scenario==1) body.back()='y'; // Matching size still requires the published hash.
            if (scenario==2) buffered.error=HTTP_ERROR_NETWORK;
            buffered.body=body.data(); buffered.body_size=body.size();
            completion(nullptr,2,&buffered,nullptr); update_update_service();
            assert(!fs::exists(download_staging_path()));
            if (scenario==3) {
                assert(dialogTitle=="Update Complete" && read(package)==payload);
                write(package,"existing package");
            } else {
                assert(dialogTitle=="Update Failed" && read(package)=="existing package");
            }
        }
        request_update_check(nullptr,reinterpret_cast<void*>(1)); complete_check(json);
        s_result.downloadSize=kMaxBufferedDownload+1;
        confirm_update_with_transport(true); update_update_service();
        assert(dialogTitle=="Update Failed" && dialogBody.find("download limit")!=std::string::npos);
        assert(read(package)=="existing package");
        // Ambiguous existing packages cannot create or replace a duplicate.
        write(temp/"mods/twilight_hd_hud.dusk","second copy");
        request_update_check(nullptr,reinterpret_cast<void*>(1)); complete_check(json);
        installAction(nullptr,0,nullptr); write(downloadPath,payload);
        completion(nullptr,3,&downloaded,nullptr); update_update_service();
        assert(dialogTitle=="Update Failed" && read(package)=="existing package");
        assert(!dialogBody.empty());
        fs::remove(temp/"mods/twilight_hd_hud.dusk");
        request_update_check(nullptr,reinterpret_cast<void*>(1)); complete_check(json);
        installAction(nullptr,0,nullptr); write(downloadPath,payload);
        completion(nullptr,4,&downloaded,nullptr); update_update_service();
        assert(dialogTitle=="Update Complete" && read(package)==payload);
        assert(!fs::exists(package.string()+".previous") && !fs::exists(package.string()+".download"));
    } else {
        assert(installAction==nullptr && downloadPath.empty());
        assert(read(package)=="existing package");
    }
    if (kCanSelfInstall) {
        const auto managerName = temp/"mods/org.twilight.hd_hud.dusk";
        fs::rename(package, managerName);
        assert(update_target() == managerName); // Online-mod installation filename.
        write(package, "duplicate");
        assert(update_target().empty());
        fs::remove(package);
        const auto relocated = temp/update_paths::from_utf8("別のフォルダ/mods");
        fs::create_directories(relocated);
        const auto movedPackage = relocated/managerName.filename();
        fs::rename(managerName, movedPackage);
        const auto scratchUtf8=(relocated/".cache/org.twilight.hd_hud/data").u8string();
        scratchPath.assign(scratchUtf8.begin(), scratchUtf8.end());
        assert(update_target() == movedPackage);
        request_update_check(nullptr,reinterpret_cast<void*>(1)); complete_check(json);
        installAction(nullptr,0,nullptr); write(downloadPath,payload);
        HttpResult relocatedDownload{}; relocatedDownload.error=HTTP_ERROR_NONE; relocatedDownload.status_code=200;
        completion(nullptr,5,&relocatedDownload,nullptr); update_update_service();
        assert(dialogTitle=="Update Complete" && read(movedPackage)==payload);
        const auto app = temp/"installation/dusklight.exe";
        fs::create_directories(app.parent_path()/"mods");
        const auto appPackage = app.parent_path()/"mods"/kReleaseAsset;
        fs::rename(movedPackage, appPackage);
        const auto roots = update_paths::roots(update_paths::from_utf8(dataPath), update_paths::from_utf8(scratchPath), app);
        assert(find_update_target(roots)==appPackage);
        auto aliases=roots;
        aliases.push_back(appPackage.parent_path());
        assert(find_update_target(aliases)==appPackage);
        fs::create_directories(temp/"mods-alias");
        fs::create_hard_link(appPackage,temp/"mods-alias"/kReleaseAsset);
        aliases.push_back(temp/"mods-alias");
        assert(find_update_target(aliases)==appPackage);
        write(relocated/kReleaseAsset,"second copy");
        assert(find_update_target(roots).empty());
        assert(s_result.installError.find("Multiple")!=std::string::npos);
        fs::remove(relocated/kReleaseAsset);
        fs::remove(appPackage);
        assert(find_update_target(roots).empty());
        assert(s_result.installError.find("could not be found")!=std::string::npos);
        const auto macRoots=update_paths::roots({}, {}, temp/"Dusklight.app/Contents/MacOS/dusklight");
        assert(macRoots.size()==2 && macRoots[0]==temp/"mods" &&
            macRoots[1]==temp/"Dusklight.app/Contents/Resources/mods");
    }
    shutdown_update_service();
    svc_http=nullptr;
    request_update_check(nullptr,reinterpret_cast<void*>(1)); update_update_service();
    assert(dialogTitle=="Update Check Failed");
    fs::remove_all(temp);
}
