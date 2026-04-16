#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <sys/stat.h>
#include <csignal>
#include <cstdlib>

using ordered_json = nlohmann::ordered_json;

static const char* FIFO_PATH  = "/tmp/eww-kbselection";
static const char* PID_PATH   = "/tmp/eww-kbselection.pid";
static const char* EWW        = "eww -c ~/.config/eww";
static std::string CACHE_FILE = std::string(getenv("HOME")) + "/.config/eww/cache/selection-info.json";

// ── helpers ──────────────────────────────────────────────────────────────────

static void ewwUpdate(const std::string& var, const std::string& val) {
    std::string cmd = std::string(EWW) + " update " + var + "='" + val + "'";
    system(cmd.c_str());
}

static std::string shellQuote(const std::string& s) {
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') out += "'\\''";
        else out += c;
    }
    return out + "'";
}

// ── selection panel ─────────────────────────────────────────────────────────

struct Panel {
    std::string              ewwKbVar;
    std::vector<std::string> entries;
    int                      sel = -1;

    void clear() { entries.clear(); sel = -1; }

    void init(const std::vector<std::string>& e) {
        entries = e;
        sel = entries.empty() ? -1 : 0;
    }

    void next() {
        if (entries.empty()) return;
        sel = (sel + 1) % (int)entries.size();
    }
    void prev() {
        if (entries.empty()) return;
        sel = (sel - 1 + (int)entries.size()) % (int)entries.size();
    }

    void selectByName(const std::string& name) {
        for (int i = 0; i < (int)entries.size(); i++) {
            if (entries[i] == name) { sel = i; return; }
        }
    }

    std::string selected() const {
        return (sel >= 0 && sel < (int)entries.size()) ? entries[sel] : "";
    }

    std::string toJson() const {
        ordered_json j;
        for (int i = 0; i < (int)entries.size(); i++)
            j[entries[i]] = (i == sel) ? "selected" : "non-selected";
        return j.dump();
    }

    void pushToEww() const { ewwUpdate(ewwKbVar, toJson()); }
};

// ── submenu loader ──────────────────────────────────────────────────────────

struct SubMenuLoader {
    ordered_json cache;

    void loadCache() {
        std::ifstream f(CACHE_FILE);
        if (f.good()) cache = ordered_json::parse(f, nullptr, false);
    }

    std::vector<std::string> getSubmenuKeys(const std::string& appName) const {
        std::vector<std::string> keys;
        if (cache.contains(appName)) {
            for (auto& item : cache[appName].items())
                keys.push_back(item.key());
        }
        return keys;
    }
};

// ── main ────────────────────────────────────────────────────────────────────

static volatile sig_atomic_t running = 1;
static void sigHandler(int) { running = 0; }

int main() {
    signal(SIGINT,  sigHandler);
    signal(SIGTERM, sigHandler);

    // Kill any existing instance
    {
        std::ifstream pf(PID_PATH);
        if (pf.good()) {
            int oldPid = 0;
            pf >> oldPid;
            if (oldPid > 0) kill(oldPid, SIGTERM);
            usleep(100000);
        }
    }

    // Write our PID
    { std::ofstream pf(PID_PATH); pf << getpid(); }

    unlink(FIFO_PATH);
    if (mkfifo(FIFO_PATH, 0666) != 0) { perror("mkfifo"); return 1; }

    int fd = open(FIFO_PATH, O_RDWR | O_NONBLOCK);
    if (fd < 0) { perror("open"); return 1; }

    Panel results;  results.ewwKbVar = "search-results-kbselection";
    Panel info;     info.ewwKbVar    = "searchinfo-kbselection";
    bool  resultPanelActive = true;
    bool  menuOpen = false;

    SubMenuLoader submenu;
    submenu.loadCache();

    auto updateSubmenuForResult = [&](const std::string& appName) {
        auto keys = submenu.getSubmenuKeys(appName);
        info.init(keys);
        ordered_json arr(keys);
        ewwUpdate("selection-info-menu", arr.dump());
        ewwUpdate("search-results-active-selection", appName);
        info.pushToEww();
    };

    struct pollfd pfd = { fd, POLLIN, 0 };
    std::string lineBuf;
    char buf[4096];

    while (running) {
        int ret = poll(&pfd, 1, 500);

        if (ret > 0 && (pfd.revents & POLLIN)) {
            int n = read(fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                lineBuf += buf;

                size_t pos;
                while ((pos = lineBuf.find('\n')) != std::string::npos) {
                    std::string line = lineBuf.substr(0, pos);
                    lineBuf.erase(0, pos + 1);
                    if (line.empty()) continue;

                    std::istringstream iss(line);
                    std::string cmd;
                    iss >> cmd;

                    // ── menu state (set by toggle script) ──────────────
                    if (cmd == "menu-opened") {
                        menuOpen = true;

                    } else if (cmd == "menu-closed") {
                        menuOpen = false;

                    // ── init-results (always processed) ─────────────────
                    } else if (cmd == "init-results") {
                        std::string rest;
                        std::getline(iss >> std::ws, rest);
                        auto arr = ordered_json::parse(rest, nullptr, false);
                        if (!arr.is_array()) continue;

                        std::vector<std::string> entries;
                        std::set<std::string> seen;
                        for (auto& e : arr) {
                            std::string s = e.get<std::string>();
                            if (seen.insert(s).second)
                                entries.push_back(s);
                        }

                        results.init(entries);
                        results.pushToEww();
                        resultPanelActive = true;
                        ewwUpdate("active-selection", "search-result-menu");

                        if (!entries.empty())
                            updateSubmenuForResult(entries[0]);

                    } else if (cmd == "clear-results") {
                        results.clear();
                        info.clear();
                        ewwUpdate(results.ewwKbVar, "");
                        ewwUpdate(info.ewwKbVar, "");

                    } else if (cmd == "reload-cache") {
                        submenu.loadCache();

                    // ── gated on menu being open ────────────────────────
                    } else if (!menuOpen) {
                        continue;   // ignore nav commands when menu is closed

                    } else if (cmd == "next" || cmd == "prev") {
                        Panel& p = resultPanelActive ? results : info;
                        if (cmd == "next") p.next(); else p.prev();
                        p.pushToEww();
                        if (resultPanelActive)
                            updateSubmenuForResult(results.selected());

                    } else if (cmd == "switch") {
                        resultPanelActive = !resultPanelActive;
                        ewwUpdate("active-selection",
                                  resultPanelActive ? "search-result-menu"
                                                    : "selection-info-menu");

                    } else if (cmd == "run") {
                        std::string sel = info.selected();
                        if (!sel.empty()) {
                            system(("setsid perl ~/.config/eww/scripts/cacheApps subrun " + shellQuote(sel) + " &>/dev/null &").c_str());
                            menuOpen = false;
                            ewwUpdate("startmenu-reveal", "false");
                            system(std::string(std::string(EWW) + " close searchbar").c_str());
                            system(std::string(std::string(EWW) + " close dismiss-layer").c_str());
                        }

                    } else if (cmd == "click-result" || cmd == "click-info") {
                        std::string entry;
                        std::getline(iss >> std::ws, entry);
                        bool isResult = (cmd == "click-result");
                        Panel& p = isResult ? results : info;

                        p.selectByName(entry);
                        p.pushToEww();

                        std::string c = isResult
                            ? "setsid perl ~/.config/eww/scripts/cacheApps run " + shellQuote(entry) + " &>/dev/null &"
                            : "setsid perl ~/.config/eww/scripts/cacheApps subrun " + shellQuote(entry) + " &>/dev/null &";
                        system(c.c_str());
                        menuOpen = false;
                        ewwUpdate("startmenu-reveal", "false");
                        system(std::string(std::string(EWW) + " close searchbar").c_str());
                        system(std::string(std::string(EWW) + " close dismiss-layer").c_str());
                    }
                }
            }
        }
    }

    close(fd);
    unlink(FIFO_PATH);
    unlink(PID_PATH);
    return 0;
}
