#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include <cstdlib>
#include <iostream>
#include <array>
#include <memory>
#include <cmath>
#include <sys/file.h>
#include <unistd.h>

using json = nlohmann::ordered_json;

struct AppConfig {
    std::string searchTerm;
    std::string launchCmd;
};

static const std::map<std::string, AppConfig> apps = {
    {"discord",          {"vesktop", "vesktop"}},
    {"code-url-handler", {"code",    "code"}}
};

static std::string exec(const std::string& cmd) {
    std::array<char, 4096> buffer;
    std::string result;
    std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return "";
    while (fgets(buffer.data(), buffer.size(), pipe.get()))
        result += buffer.data();
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: taskbar-ctl <app-name>\n";
        return 1;
    }

    // Lock so spam clicks don't pile up Python processes
    int lockfd = open("/tmp/taskbar-ctl.lock", O_CREAT | O_RDWR, 0666);
    if (lockfd >= 0 && flock(lockfd, LOCK_EX | LOCK_NB) != 0) return 0; // another instance running

    std::string appName = argv[1];
    auto it = apps.find(appName);
    if (it == apps.end()) {
        std::cerr << "Unknown app: " << appName << "\n";
        return 1;
    }

    const auto& config = it->second;

    // Search for open views matching this app
    std::string searchResult = exec("wfctl \"search views\" " + config.searchTerm + " 2>/dev/null");

    json views = json::parse(searchResult, nullptr, false);
    if (views.is_discarded() || !views.is_array() || views.empty()) {
        system(("setsid " + config.launchCmd + " &>/dev/null &").c_str());
        return 0;
    }

    // Pick the most recently focused view
    json* bestView = nullptr;
    long long bestTimestamp = -1;
    for (auto& view : views) {
        long long ts = view.value("last-focus-timestamp", (long long)0);
        if (ts > bestTimestamp) {
            bestTimestamp = ts;
            bestView = &view;
        }
    }

    if (!bestView) {
        system(("setsid " + config.launchCmd + " &>/dev/null &").c_str());
        return 0;
    }

    int viewId       = (*bestView)["id"].get<int>();
    bool minimized   = (*bestView)["minimized"].get<bool>();
    int viewX        = (*bestView)["base-geometry"]["x"].get<int>();
    int viewY        = (*bestView)["base-geometry"]["y"].get<int>();

    // Get current workspace coordinates from the primary output
    std::string wsetResult = exec("wfctl \"list wsets\" 2>/dev/null");
    json wsets = json::parse(wsetResult, nullptr, false);

    int curWsX = 0, curWsY = 0, gridW = 3;
    int outWidth = 1920, outHeight = 1080;
    if (!wsets.is_discarded() && wsets.is_array()) {
        for (auto& ws : wsets) {
            if (ws.value("output-name", "") == "DP-1") {
                curWsX = ws["workspace"]["x"].get<int>();
                curWsY = ws["workspace"]["y"].get<int>();
                gridW  = ws["workspace"]["grid_width"].get<int>();
                break;
            }
        }
    }

    bool onCurrentWs = (viewX >= 0 && viewX < outWidth &&
                        viewY >= 0 && viewY < outHeight);

    std::string id = std::to_string(viewId);

    static const char* WF_ENV = "WAYFIRE_SOCKET=$WAYFIRE_SOCKET ";
    static const char* WF_CTL = "wfctl ";

    if (onCurrentWs) {
        std::string cmd = std::string(WF_ENV) + WF_CTL + "\"minimize view\" " + id + (minimized ? " false" : " true");
        system(cmd.c_str());
    } else {
        int vWsX = curWsX + (int)floor((double)viewX / outWidth);
        int vWsY = curWsY + (int)floor((double)viewY / outHeight);
        std::string cmd = std::string(WF_ENV) + WF_CTL + "\"set workspace\" " +
            std::to_string(vWsY * gridW + vWsX + 1);
        system(cmd.c_str());
    }

    return 0;
}
