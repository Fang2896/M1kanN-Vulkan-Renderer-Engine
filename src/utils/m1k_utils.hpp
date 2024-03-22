//
// Created by fangl on 2023/11/29.
//

#pragma once

#include <string>
#include <algorithm>
#include <fstream>
#include <cstddef>
#include <algorithm>
#include <filesystem>
#include <iostream>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>


namespace std {

}

namespace m1k {

// from: https://stackoverflow.com/a/57595105
template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hashCombine(seed, rest), ...);
}

struct TransformComponent {
    glm::vec3 translation{};    // position offset
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 rotation{};

    // faster mat4 transform
    glm::mat4 mat4();

    glm::mat3 normalMatrix();
};


std::string getFileExtension(const std::string& filePath);

bool identifyFileSuffix(const std::string& suffix, const std::string& filePath);
std::string getFilePathNameWithoutSuffix(const std::string& path);

bool readFileBinary(const std::string& filepath, char** data, size_t* size);
void writeFileBinary(const std::string& file_path, const void* data, size_t size);
bool createDirectoryIfNotExists(const std::string& path);

void* allocateAligned(size_t size, size_t alignment);
void deallocateAligned(void* aligned_ptr);

}
