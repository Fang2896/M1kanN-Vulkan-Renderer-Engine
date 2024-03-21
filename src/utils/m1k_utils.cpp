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


glm::mat4 TransformComponent::mat4() {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x + glm::radians(180.0f));
    const float s2 = glm::sin(rotation.x + glm::radians(180.0f));
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    return glm::mat4{
        {
            scale.x * (c1 * c3 + s1 * s2 * s3),
            scale.x * (c2 * s3),
            scale.x * (c1 * s2 * s3 - c3 * s1),
            0.0f,
        },
        {
            scale.y * (c3 * s1 * s2 - c1 * s3),
            scale.y * (c2 * c3),
            scale.y * (c1 * c3 * s2 + s1 * s3),
            0.0f,
        },
        {
            scale.z * (c2 * s1),
            scale.z * (-s2),
            scale.z * (c1 * c2),
            0.0f,
        },
        {translation.x, translation.y, translation.z, 1.0f}};
}

glm::mat3 TransformComponent::normalMatrix() {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 inv_scale = 1.0f / scale;

    return glm::mat3{
        {
            inv_scale.x * (c1 * c3 + s1 * s2 * s3),
            inv_scale.x * (c2 * s3),
            inv_scale.x * (c1 * s2 * s3 - c3 * s1),
        },
        {
            inv_scale.y * (c3 * s1 * s2 - c1 * s3),
            inv_scale.y * (c2 * c3),
            inv_scale.y * (c1 * c3 * s2 + s1 * s3),
        },
        {
            inv_scale.z * (c2 * s1),
            inv_scale.z * (-s2),
            inv_scale.z * (c1 * c2),
        }

    };
}


}


