#include <string>
#include <regex>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>

class GLSLConverter {
private:
    static std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }
        return std::string(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        );
    }
    static void writeFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::trunc);
        if (!file.is_open()) {
            throw std::runtime_error("Could not write file: " + filename);
        }
        file << content;
    }
    static std::string trimExtraNewlines(const std::string& input) {
        std::string output = input;
        std::regex multipleNewlines("\n\\s*\n");
        output = std::regex_replace(output, multipleNewlines, "\n");
        return output;
    }
    static std::string extractAndRemove(std::string& input, const std::regex& pattern) {
        std::stringstream extracted;
        std::smatch match;
        while (std::regex_search(input, match, pattern)) {
            extracted << match.str();
            input = match.prefix().str() + match.suffix().str();
        }
        return extracted.str();
    }
    static std::string removeGLExtensions(const std::string& input) {
        std::regex extension_block("#ifdef\\s+GL_ARB_shading_language_420pack[\\s\\S]*?#endif", std::regex_constants::ECMAScript);//idk what this is but it has 420 so i don't trust it
        return std::regex_replace(input, extension_block, "");
    }
    static std::string fixUniformReferences(const std::string& input) {
        std::string result = input;
        std::regex struct_ref("(_\\d+)\\.");
        result = std::regex_replace(result, struct_ref, "");
        return result;
    }
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }
public:
    static std::string convert(const std::string& input) {
        std::stringstream output;
        std::string working = input;
        std::regex version_pattern("#version\\s+\\d+\\s+\\w*");
        working = std::regex_replace(working, version_pattern, "");
        working = removeGLExtensions(working);
        std::stringstream uniforms;
        std::regex block_pattern("layout\\s*\\([^)]*\\)\\s*uniform\\s+(\\w+)\\s*\\{([^}]+)\\}\\s*(\\w+)\\s*;");
        std::regex uniform_pattern("\\s*(\\w+)\\s+(\\w+)\\s*;");
        std::regex layout_uniform_pattern("layout\\s*\\([^)]*\\)\\s*uniform\\s+(\\w+)\\s+(\\w+)\\s*;");
        std::smatch match;
        while (std::regex_search(working, match, block_pattern)) {
            std::string block_content = match[2].str();
            std::string::const_iterator block_search(block_content.cbegin());
            std::smatch uniform_match;
            while (std::regex_search(block_search, block_content.cend(), uniform_match, uniform_pattern)) {
                std::string type = uniform_match[1].str();
                std::string name = uniform_match[2].str();
                type.erase(std::remove(type.begin(), type.end(), '*'), type.end());
                name.erase(std::remove(name.begin(), name.end(), '*'), name.end());
                uniforms << "uniform " << type << " " << name << ";\n";
                block_search = uniform_match.suffix().first;
            }
            working = match.prefix().str() + match.suffix().str();
        }
        while (std::regex_search(working, match, layout_uniform_pattern)) {
            std::string type = match[1].str();
            std::string name = match[2].str();
            type.erase(std::remove(type.begin(), type.end(), '*'), type.end());
            name.erase(std::remove(name.begin(), name.end(), '*'), name.end());
            uniforms << "uniform " << type << " " << name << ";\n";
            working = match.prefix().str() + match.suffix().str();
        }
        working = fixUniformReferences(working);
        working = trim(working);
        output << "#version 330 core\n";
        if (uniforms.str().length() > 0) {output << trim(uniforms.str()) << "\n";}
        if (!working.empty()) {output << "\n" << working;}
        return trimExtraNewlines(output.str());
    }

    static void convertFile(const std::string& filename) {
        try {
            std::string input = readFile(filename);
            std::string converted = convert(input);
            writeFile(filename, converted);
        } catch (const std::exception& e) {
            std::cerr << "error: " << e.what() << std::endl;
            throw;
        }
    }
};
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " shaderfile._s ...\n";
        return 1;
    }
    try {
        for (int i = 1; i < argc; i++) {
            std::string inputFile = argv[i];
            //std::cout << "converting " << inputFile << "...\n";
            GLSLConverter::convertFile(inputFile);
        }
    } catch (const std::exception& e) {
        std::cerr << "failure: " << e.what() << "\n";
        return 1;
    }

    return 0;
}