#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <nlohmann/json.hpp>

using ordered_json = nlohmann::ordered_json;

int main(int argc, char *argv[]) {
    // Join all arguments into a single string with spaces
    std::string argString;
    for (int i = 1; i < argc; i++) {
        argString += argv[i];
        if (i != argc - 1) {
            argString += " ";
        }
    }

    std::string command1 = "~/.config/eww/scripts/active-selection-info-menu-init.out '" + argString + "'";
    std::string command2 = "eww -c ~/.config/eww update search-results-active-selection='" + argString + "'";
    std::system(command1.c_str());
    std::system(command2.c_str());


    // Path to your JSON file
    std::string filePath = std::string(getenv("HOME")) + "/.config/eww/cache/selection-info.json";

    // Load JSON
    std::ifstream ifs(filePath);
    ordered_json json;
    ifs >> json;

    // Navigate down json structure using argString as key
    auto subJson = json[argString];

    // Create a JSON array from keys in subJson
    ordered_json::array_t jsonArray;
    for (const auto& item : subJson.items()) {
        jsonArray.push_back(item.key());
    }

    // Convert jsonArray into a string
    nlohmann::ordered_json jsonX(jsonArray);
    std::string output = jsonX.dump();

    // Print the resulting JSON array of keys
    std::string command = "eww -c ~/.config/eww/ update selection-info-menu='" + output + "'";

    // Execute shell command
    std::system(command.c_str());

    return 0;
}
