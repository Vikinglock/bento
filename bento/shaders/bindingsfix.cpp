#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>

int main(int argc, char* argv[]) {

    std::ifstream gf(argv[1]);
    if (!gf.is_open()) {
        std::cerr << "cannot open file: " << argv[1] << std::endl;
        return 0;//                                                 muahahahahah
    }
    std::string glsl((std::istreambuf_iterator<char>(gf)), std::istreambuf_iterator<char>());

    std::ifstream mf(argv[2]);
    if (!mf.is_open()) {std::cerr << "cannot open file: " << argv[2] << std::endl;return 0;}
    std::string metal((std::istreambuf_iterator<char>(mf)), std::istreambuf_iterator<char>());

    std::unordered_map<std::string, int> bindings;
    std::regex pattern("(layout\\(set\\s*=\\s*0,\\s*binding\\s*=\\s*(\\d+)\\)\\s*uniform\\s*(\\w+))");
    std::smatch match;
    std::string::const_iterator search_start = glsl.cbegin();

    while (regex_search(search_start, glsl.cend(), match, pattern)) {
        int binding = std::stoi(match[2]);
        bindings[match[1]] = binding;

        search_start = match.suffix().first;
    }
    std::string newmetal = metal;
    std::regex buffer_pattern("\\[\\[buffer\\((\\d+)\\)\\]\\]");
    std::regex param_pattern("(constant\\s+)(\\w+)(&\\s+\\w+\\s*\\[\\[buffer\\()\\d+(\\)\\]\\])");
    for (std::unordered_map<std::string, int>::const_iterator it = bindings.begin(); it != bindings.end(); ++it) {
        const std::string& uniform_name = it->first;
        int binding = it->second;
        std::regex name_pattern("uniform\\s+(\\w+)");
        std::smatch name_match;
        if (std::regex_search(uniform_name, name_match, name_pattern)) {
            std::string struct_name = name_match[1];
            std::string struct_pattern = "\\[\\[buffer\\(\\d+\\)\\]\\]\\s*" + struct_name;
            std::regex metal_struct_pattern(struct_pattern);
            std::smatch metal_match;
            if (std::regex_search(newmetal, metal_match, metal_struct_pattern)) {
                std::string full_match = metal_match[0];
                std::string new_attribute = "[[buffer(" + std::to_string(binding) + ")]] " + struct_name;
                size_t pos = newmetal.find(full_match);
                if (pos != std::string::npos) {
                    newmetal.replace(pos, full_match.length(), new_attribute);
                }
            }
            std::string search_str = newmetal;
            std::smatch param_match;
            while (std::regex_search(search_str, param_match, param_pattern)) {
                std::string type_name = param_match[2].str();
                if (type_name == struct_name) {
                    std::string full_match = param_match[0].str();
                    std::string new_param = param_match[1].str()+type_name+param_match[3].str()+std::to_string(binding)+param_match[4].str();                    
                    size_t pos = newmetal.find(full_match);
                    if (pos != std::string::npos) {
                        newmetal.replace(pos, full_match.length(), new_param);
                    }
                }
                search_str = param_match.suffix().str();
            }
        }
    }



    std::ofstream metal_file(argv[2]);
    if (metal_file.is_open()) {
        metal_file << newmetal;
        metal_file.close();
    } else {
        std::cerr << "cannot open file: " << argv[2] << std::endl;
        return 1;
    }

    return 0;
}