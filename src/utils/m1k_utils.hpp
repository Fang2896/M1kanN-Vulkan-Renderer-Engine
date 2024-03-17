//
// Created by fangl on 2023/11/29.
//

#pragma once

#include <string>
#include <algorithm>
#include <glm/vec3.hpp>


namespace std {

}

namespace m1k {

// from: https://stackoverflow.com/a/57595105
template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
}

std::string getFileExtension(const std::string& filePath);

bool identifyFileSuffix(const std::string& suffix, const std::string& filePath);

}
