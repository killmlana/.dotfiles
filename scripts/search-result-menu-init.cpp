#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <set>  // For std::set to track duplicates

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <json_array_string>" << std::endl;
        return 1;
    }

    try {
        // Debug: Show input
        std::cout << "Input JSON array string: " << argv[1] << std::endl;

        // Parse the input JSON array string directly
        auto jsonArray = nlohmann::ordered_json::parse(argv[1]);

        // Verify that jsonArray is indeed an array
        if (!jsonArray.is_array()) {
            std::cerr << "Error: Input is not a valid JSON array." << std::endl;
            return 1;
        }

        // Debug: Show parsed JSON array
        std::cout << "Parsed JSON array: " << jsonArray.dump() << std::endl;

        // Get the initial value as the first element of the array
        std::string initial_value = jsonArray[0].get<std::string>();

        // Debug: Show initial value
        std::cout << "Initial value to be set as selected: " << initial_value << std::endl;

        // Prepare an ordered JSON object to store the results
        nlohmann::ordered_json orderedJson;

        // Set to keep track of added items to avoid duplicates
        std::set<std::string> seenItems;

        for (size_t i = 0; i < jsonArray.size(); ++i) {
            std::string item = jsonArray[i].get<std::string>();

            // Check if item has already been processed
            if (seenItems.find(item) == seenItems.end()) {
                orderedJson[item] = (i == 0 ? "selected" : "non-selected");
                seenItems.insert(item);

                // Debug: Show current state of orderedJson
                std::cout << "Processing item: " << item << " - Status: " << orderedJson[item] << std::endl;
            } else {
                // Debug: Duplicate item found
                std::cout << "Duplicate item found and skipped: " << item << std::endl;
            }
        }

        // Print out the created JSON object
        std::string jsonString = orderedJson.dump();
        std::cout << "Created JSON object for command: " << jsonString << std::endl;

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

        // Quote initial_value to handle spaces and special characters
        std::string quoted_initial_value = "'";
        for (char c : initial_value) {
            if (c == '\'') {
                quoted_initial_value += "'\\''"; // Escape single quote for shell
            } else {
                quoted_initial_value += c;
            }
        }
        quoted_initial_value += "'";

        // Prepare command strings with properly quoted values
        std::string command1 = "eww -c ~/.config/eww/ update search-results-kbselection=" + escapedJsonString;
        std::string command2 = "~/.config/eww/scripts/search-info-menu-init.out " + quoted_initial_value;
        std::string command3 = "~/.config/eww/scripts/active-selection-info-menu-init.out '" + quoted_initial_value + "'";

        // Debug: Show commands to be executed
        std::cout << "Command 1: " << command1 << std::endl;
        std::cout << "Command 2: " << command2 << std::endl;

        // Execute shell commands
        std::system(command1.c_str());
        std::system(command2.c_str());
        std::system(command3.c_str());
    } catch (const nlohmann::json::parse_error &e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
