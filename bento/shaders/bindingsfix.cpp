#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <unordered_map>

std::string read_file(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(1);  // Exit if file cannot be opened
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

// Function to extract uniforms and their bindings from GLSL code
std::unordered_map<std::string, int> extract_uniform_bindings(const std::string& glsl_code) {
    std::unordered_map<std::string, int> bindings;
    std::regex pattern("(layout\\(set\\s*=\\s*0,\\s*binding\\s*=\\s*(\\d+)\\)\\s*uniform\\s*(\\w+))");
    std::smatch match;
    std::string::const_iterator search_start = glsl_code.cbegin();

    while (regex_search(search_start, glsl_code.cend(), match, pattern)) {
        int binding = std::stoi(match[2]);
        bindings[match[1]] = binding;

        search_start = match.suffix().first;
    }

    return bindings;
}

std::string update_metal_code(const std::string& metal_code, 
                            const std::unordered_map<std::string, int>& bindings) {
    std::string updated_code = metal_code;
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
            if (std::regex_search(updated_code, metal_match, metal_struct_pattern)) {
                std::string full_match = metal_match[0];
                std::string new_attribute = "[[buffer(" + std::to_string(binding) + ")]] " + struct_name;
                size_t pos = updated_code.find(full_match);
                if (pos != std::string::npos) {
                    updated_code.replace(pos, full_match.length(), new_attribute);
                }
            }
            std::string search_str = updated_code;
            std::smatch param_match;
            while (std::regex_search(search_str, param_match, param_pattern)) {
                std::string type_name = param_match[2].str();
                if (type_name == struct_name) {
                    std::string full_match = param_match[0].str();
                    std::string new_param = param_match[1].str()+type_name+param_match[3].str()+std::to_string(binding)+param_match[4].str();                    
                    size_t pos = updated_code.find(full_match);
                    if (pos != std::string::npos) {
                        updated_code.replace(pos, full_match.length(), new_param);
                    }
                }
                search_str = param_match.suffix().str();
            }
        }
    }
    
    return updated_code;
}



int main(int argc, char* argv[]) {
    std::string glsl_filename = argv[1];
    std::string metal_filename = argv[2];
    std::string glsl_code = read_file(glsl_filename);
    std::string metal_code = read_file(metal_filename);
    std::unordered_map<std::string, int> bindings = extract_uniform_bindings(glsl_code);
    std::string updated_metal_code = update_metal_code(metal_code, bindings);
    std::ofstream metal_file(metal_filename);
    if (metal_file.is_open()) {
        metal_file << updated_metal_code;
        metal_file.close();
    } else {
        std::cerr << "Error opening file for writing: " << metal_filename << std::endl;
        return 1;
    }

    return 0;
}