#include <string>
#include <regex>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " shaderfile._s ...\n";
        return 1;
    }
    for (int i = 1; i < argc; i++) {
        std::ifstream f(argv[i]);
        std::string file = std::string(std::istreambuf_iterator<char>(f),std::istreambuf_iterator<char>());

        std::stringstream output;
        std::string working = file;

        //\\s* any amount of spaces i presume
        //\\d+ any number i presume
        //this reminds me of javascript

        std::regex ekstensionBlok("#ifdef\\s+GL_ARB_shading_language_420pack[\\s\\S]*?#endif");
        working = std::regex_replace(working, ekstensionBlok, "");
        std::regex bersionPattern("#version 330");
        working = std::regex_replace(working, bersionPattern,"");
        std::stringstream uniforms;
        std::regex block_pattern("layout\\s*\\([^)]*\\)\\s*uniform\\s+(\\w+)\\s*\\{([^}]+)\\}\\s*(\\w+)\\s*;");
        std::regex uniform_pattern("\\s*(\\w+)\\s+(\\w+(?:\\s*\\[\\d+\\])?)\\s*;");
        std::regex layout_uniform_pattern("layout\\s*\\([^)]*\\)\\s*uniform\\s+(\\w+)\\s+(\\w+(?:\\s*\\[\\d+\\])?)\\s*;");
        std::smatch match;
        while (std::regex_search(working, match, block_pattern)) {
            std::string block_content = match[2].str();
            std::string::const_iterator block_search(block_content.cbegin());
            std::smatch uniform_match;
            while (std::regex_search(block_search, block_content.cend(), uniform_match, uniform_pattern)) {
                std::string type = uniform_match[1].str();
                std::string name = uniform_match[2].str();
                std::string cleanType = type;
                std::string cleanName = name;
                cleanType.erase(std::remove(cleanType.begin(), cleanType.end(), '*'), cleanType.end());
                cleanName.erase(std::remove(cleanName.begin(), cleanName.end(), '*'), cleanName.end());
                std::regex arrayPattern("(\\w+)\\s*\\[(\\d+)\\]");
                std::smatch arrayMatch;
                if (std::regex_search(cleanName, arrayMatch, arrayPattern)) uniforms << "uniform " + cleanType + " " + arrayMatch[1].str() + "[" + arrayMatch[2].str() + "];\n";
                else uniforms << "uniform " + cleanType + " " + cleanName + ";\n";
                block_search = uniform_match.suffix().first;
            }
            working = match.prefix().str() + match.suffix().str();
        }
        while (std::regex_search(working, match, layout_uniform_pattern)) {
            std::string type = match[1].str();
            std::string name = match[2].str();
            std::string cleanType = type;
            std::string cleanName = name;
            cleanType.erase(std::remove(cleanType.begin(), cleanType.end(), '*'), cleanType.end());
            cleanName.erase(std::remove(cleanName.begin(), cleanName.end(), '*'), cleanName.end());
            std::regex arrayPattern("(\\w+)\\s*\\[(\\d+)\\]");
            std::smatch arrayMatch;
            if (std::regex_search(cleanName, arrayMatch, arrayPattern)) uniforms << "uniform " + cleanType + " " + arrayMatch[1].str() + "[" + arrayMatch[2].str() + "];\n";
            else uniforms << "uniform " + cleanType + " " + cleanName + ";\n";
            working = match.prefix().str() + match.suffix().str();
        }
        output<<"#version 330 core\n";

        std::regex strrref("(_\\d+)\\.");
        working = std::regex_replace(working, strrref, "");

        size_t first = working.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) working = "";
        size_t last = working.find_last_not_of(" \t\n\r");
        working = working.substr(first, last - first + 1);

        if (uniforms.str().length() > 0){

            size_t first = uniforms.str().find_first_not_of(" \t\n\r");
            if (first == std::string::npos) output <<  "" << "\n";
            size_t last = uniforms.str().find_last_not_of(" \t\n\r");
            output <<  uniforms.str().substr(first, last - first + 1) << "\n";
        }
        if (!working.empty())output << "\n" << working;
        std::regex newlines("\n\\s*\n");
        file = std::regex_replace(output.str(), newlines, "\n");

        
        std::ofstream of(argv[i], std::ios::trunc);
        of << file;
    }
    return 0;
}