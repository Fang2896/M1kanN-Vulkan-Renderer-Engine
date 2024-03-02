//
// Created by fangl on 2024/2/27.
//

#include "m1k_utils.hpp"

namespace m1k {

std::string getFileExtension(const std::string& filePath) {
    std::size_t dotPos = filePath.rfind('.');
    if(dotPos != std::string::npos) {
        return filePath.substr(dotPos + 1);
    }
    return "";
}

bool identifyFileSuffix(const std::string& suffix, const std::string& filePath) {
    std::string extension = getFileExtension(filePath);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    return extension == suffix;
}

}


