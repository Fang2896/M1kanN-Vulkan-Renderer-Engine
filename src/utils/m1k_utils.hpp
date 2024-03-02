//
// Created by fangl on 2023/11/29.
//

#pragma once

#include <string>
#include <algorithm>


namespace m1k {

std::string getFileExtension(const std::string& filePath);

bool identifyFileSuffix(const std::string& suffix, const std::string& filePath);

}
