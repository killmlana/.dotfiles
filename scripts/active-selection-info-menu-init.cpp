#include <iostream>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

using ordered_json = nlohmann::ordered_json;

// Function to read JSON file
ordered_json readJsonFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filename);
    }

    ordered_json jsonObject;
    file >> jsonObject;
    return jsonObject;
}

// Function to create a new ordered JSON based on the provided key
ordered_json createSelectedJson(const ordered_json& jsonObject, const std::string& key) {
    ordered_json selectedJson;

    // Check if the key exists in the JSON object
    if (jsonObject.contains(key)) {
        // Get the sub-object corresponding to the key
        const ordered_json& subObject = jsonObject.at(key);

        // Initialize a flag to mark the first sub-key
        bool first = true;

        // Iterate over the sub-keys and construct the new JSON object
        for (const auto& item : subObject.items()) {
            selectedJson[item.key()] = first ? "selected" : "non-selected";
            first = false;  // Only the first item should be "selected"
        }
    } else {
        std::cerr << "Key not found in JSON file: " << key << std::endl;
    }

    return selectedJson;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " \"key\"" << std::endl;
        return 1;
    }

    // Retrieve the key from command-line arguments
    std::string key = argv[1];

    try {
        // Read the JSON file
        ordered_json jsonObject = readJsonFile(std::string(getenv("HOME")) + "/.config/eww/cache/selection-info.json");

        // Create the new ordered JSON based on the provided key
        ordered_json selectedJson = createSelectedJson(jsonObject, key);
        std::string jsonString = selectedJson.dump();

        // Escape single quotes in jsonString to prevent breaking the shell command
        std::string escapedJsonString = "'";
        for (char c : jsonString) {
            if (c == '\'') {
                escapedJsonString += "'\\''"; // Escape single quote for shell
            } else {
                escapedJsonString += c;
            }
        }
        escapedJsonString += "'";

        // Output the new JSON object
        std::string command = "eww -c ~/.config/eww update searchinfo-kbselection=" + escapedJsonString;
        std::system(command.c_str());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
