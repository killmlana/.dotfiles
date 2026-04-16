#include <nlohmann/json.hpp>
#include <cstdlib>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include <unistd.h>


std::string execAndGetOutput(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    
    // Explicitly specify the type for unique_ptr
    std::unique_ptr<FILE, int(*)(FILE*)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

std::vector<std::pair<std::string, std::string>> ConvertJsonToList(const std::string& jsonString) {
    std::vector<std::pair<std::string, std::string>> list;

    auto jsonObj = nlohmann::ordered_json::parse(jsonString);

    for(auto& pair : jsonObj.items()) {
        list.push_back(std::make_pair(pair.key(), pair.value()));
    }

    return list;
}

std::string changeSelection(std::vector<std::pair<std::string, std::string>>& list, bool forward)
{
    std::string newSelection;
    // Find the currently selected item
    for(auto it = list.begin(); it != list.end(); ++it)
    {
        if(it->second == "selected")
        {
            // Set the current item to "non-selected"
            it->second = "non-selected";

            // Select the next or previous item
            if(forward)
            {
                if(std::next(it) != list.end())
                    newSelection = std::next(it)->first;
                else
                    newSelection = list.begin()->first;
            }
            else
            {
                if(it == list.begin())
                    newSelection = list.rbegin()->first;
                else
                    newSelection = std::prev(it)->first;
            }

            // Set the new selected item
            for(auto& item : list)
            {
                if(item.first == newSelection)
                {
                    item.second = "selected";
                    break;
                }
            }

            // Stop searching since we've found our selected item
            break;
        }
    }

    // Call the script with the new selected value
    std::cout<<newSelection;
    return newSelection;
    //std::string command2 = "~/.config/eww/scripts/active-selection-info-menu-init.out '" + newSelection + "'";
    //std::string command1 = "~/.config/eww/scripts/search-info-menu-init.out '" + newSelection + "'";
    //system(command1.c_str());
    //system(command2.c_str());
}

int main(int argc, char *argv[]) {
    // Check there is at least one command-line parameter
    if (argc > 1) {
        // Run the command and get the JSON output
        std::string jsonString; //jsonString from the command output
        std::string selection = execAndGetOutput("eww -c ~/.config/eww get active-selection");
        bool isfound = selection.find("search-result-menu") != std::string::npos;


        if (isfound) jsonString = execAndGetOutput("eww -c ~/.config/eww get search-results-kbselection");
        else jsonString = execAndGetOutput("eww -c ~/.config/eww get searchinfo-kbselection");
        if (jsonString == "") {
            return 1;
        }
        if (true//execAndGetOutput("eww -c ~/.config/eww get startmenu-reveal") == "true" TODO: fix this.
        ) {
            // Convert JSON string to a list
            auto list = ConvertJsonToList(jsonString);

            // Check the first command-line argument and move the selection accordingly
            std::string direction = argv[1];  // First command-line argument
            if (direction == "next") {
                std::string newSelection = changeSelection(list, true);
                // Convert list back to json
                nlohmann::ordered_json updatedJson;
                for(const auto& pair : list)
                    updatedJson[pair.first] = pair.second;
    
                // Convert json to string
                std::string updatedJsonString = updatedJson.dump();
                std::cout<<updatedJsonString;
                
                std::string command1;
                std::string command2;
   
                // Execute the command
                if (isfound) {
                    command1 = "eww -c ~/.config/eww update search-results-kbselection=\'" + updatedJsonString + "\'";
                    command2 = "~/.config/eww/scripts/search-info-menu-init.out '" + newSelection + "'";
                }
                else {
                    command1 = "eww -c ~/.config/eww update searchinfo-kbselection=\'" + updatedJsonString + "\'";
                }
                system(command1.c_str());
                system(command2.c_str());


            } else if (direction == "prev") {
                std::string newSelection = changeSelection(list, false);
                // Convert list back to json
                nlohmann::ordered_json updatedJson;
                for(const auto& pair : list)
                    updatedJson[pair.first] = pair.second;
    
                // Convert json to string
                std::string updatedJsonString = updatedJson.dump();

                // Formulate the complete command with updated json 
                std::string command1;
                std::string command2;
   
                // Execute the command
                if (isfound) {
                    command1 = "eww -c ~/.config/eww update search-results-kbselection=\'" + updatedJsonString + "\'";
                    command2 = "~/.config/eww/scripts/search-info-menu-init.out '" + newSelection + "'";
                }
                else {
                    command1 = "eww -c ~/.config/eww update searchinfo-kbselection=\'" + updatedJsonString + "\'";
                }
                system(command1.c_str());
                system(command2.c_str());


            } else if (direction == "switch") {
                std::string selection = execAndGetOutput("eww -c ~/.config/eww get active-selection");
                if (isfound) {
                    system("eww -c ~/.config/eww update active-selection='selection-info-menu'");
                }
                else  {
                    system("eww -c ~/.config/eww update active-selection='search-result-menu'");
                }

            } else if (direction == "run") {
                // Find the currently selected entry and run it
                for (const auto& pair : list) {
                    if (pair.second == "selected") {
                        std::string command;
                        if (isfound) {
                            command = "perl ~/.config/eww/scripts/cacheApps run '" + pair.first + "'";
                        } else {
                            command = "perl ~/.config/eww/scripts/cacheApps subrun '" + pair.first + "'";
                        }
                        system(command.c_str());
                        system("~/.config/eww/scripts/toggle-startmenu");
                        break;
                    }
                }

            } else if ((direction == "hover-result" || direction == "hover-info") && argc > 2) {
                std::string target = argv[2];
                bool isResult = (direction == "hover-result");

                // Debounce: write target, wait 30ms, bail if a newer hover overwrote it
                std::string debounceFile = "/tmp/eww-hover-" + std::string(isResult ? "result" : "info");
                { std::ofstream out(debounceFile); out << target; }
                usleep(30000);
                { std::ifstream in(debounceFile); std::string cur; std::getline(in, cur);
                  if (cur != target) return 0; }

                // Read the correct kbselection variable
                std::string hoverJson = execAndGetOutput(
                    isResult ? "eww -c ~/.config/eww get search-results-kbselection"
                             : "eww -c ~/.config/eww get searchinfo-kbselection");
                if (hoverJson.empty()) return 1;

                auto hoverList = ConvertJsonToList(hoverJson);

                // Set all to non-selected, target to selected
                nlohmann::ordered_json updatedJson;
                for (auto& pair : hoverList)
                    updatedJson[pair.first] = (pair.first == target) ? "selected" : "non-selected";

                std::string updatedJsonString = updatedJson.dump();

                if (isResult) {
                    system(("eww -c ~/.config/eww update search-results-kbselection='" + updatedJsonString + "'").c_str());
                    system("eww -c ~/.config/eww update active-selection='search-result-menu'");
                    system(("~/.config/eww/scripts/search-info-menu-init.out '" + target + "'").c_str());
                } else {
                    system(("eww -c ~/.config/eww update searchinfo-kbselection='" + updatedJsonString + "'").c_str());
                    system("eww -c ~/.config/eww update active-selection='selection-info-menu'");
                }

            } else {
                std::cerr << "Invalid argument.\n";
                return 1;
            }

        }
    }
    else {
        std::cerr << "Usage: a.out <next|prev|switch|run|hover-result|hover-info> [entry]\n";
        return 1;
    }
}