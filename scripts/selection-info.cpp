#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>
#include "inipp/inipp.h"

nlohmann::ordered_json parseDesktop(const std::string& filename) {
    std::ifstream is(filename);
    inipp::Ini<char> ini;

    try {
        ini.parse(is);
    } catch (const std::exception& e) {
        std::cerr << "Error parsing file " << filename << ": " << e.what() << std::endl;
        return {}; // Return an empty JSON object if parsing fails
    }

    std::string mainAppName = ini.sections["Desktop Entry"]["Name"];
    std::string mainAppNameinner = "Launch " + mainAppName;
    std::string mainAppComm = ini.sections["Desktop Entry"]["Exec"];
    bool isTerminal = ini.sections["Desktop Entry"]["Terminal"] == "true";

    nlohmann::ordered_json json = {{mainAppName, {}}};

    json[mainAppName][mainAppNameinner] = isTerminal ? "footclient " + mainAppComm : mainAppComm;

    for (auto& sec : ini.sections) {
        if (sec.first != "Desktop Entry") {
            std::string innerAppName = sec.second["Name"];
            std::string innerAppComm = sec.second["Exec"];

            json[mainAppName][innerAppName] = isTerminal ? "footclient " + innerAppComm : innerAppComm;
        }
    }

    return json;
}


int main() {
    nlohmann::ordered_json finalOutput;

    boost::filesystem::path path = "/usr/share/applications";

    if(boost::filesystem::exists(path) && boost::filesystem::is_directory(path)) {
        for(boost::filesystem::directory_entry& x : boost::filesystem::directory_iterator(path)){
            if(x.path().extension() == ".desktop") {
                nlohmann::ordered_json fileJson = parseDesktop(x.path().string());
                finalOutput.update(fileJson);
            }
        }
    }

    std::string cachePath = std::string(getenv("HOME")) + "/.config/eww/cache/selection-info.json";
    std::ofstream o(cachePath);
    o << std::setw(4) << finalOutput << std::endl;

    return 0;
}
