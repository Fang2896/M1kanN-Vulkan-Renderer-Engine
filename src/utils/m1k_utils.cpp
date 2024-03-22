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

std::string getFilePathNameWithoutSuffix(const std::string& path) {
    size_t last_slash_pos = path.find_last_of("/\\");
    if (last_slash_pos == std::string::npos) {
        last_slash_pos = 0;
    } else {
        last_slash_pos += 1;
    }

    size_t last_dot_pos = path.find_last_of(".");
    if (last_dot_pos != std::string::npos && last_dot_pos < last_slash_pos) {
        last_dot_pos = std::string::npos;
    }

    if (last_dot_pos == std::string::npos) {
        return path.substr(last_slash_pos);
    }

    return path.substr(last_slash_pos, last_dot_pos - last_slash_pos);
}

bool readFileBinary(const std::string& filepath, char** data, size_t* size) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        *data = nullptr;
        *size = 0;
        return false;
    }

    file.seekg(0, std::ios::end);
    *size = file.tellg();
    file.seekg(0, std::ios::beg);

    *data = new char[*size];
    if (file.read(*data, *size)) {
        return true;
    } else {
        delete[] *data;
        *data = nullptr;
        *size = 0;
        return false;
    }
}

void writeFileBinary(const std::string& file_path, const void* data, size_t size) {
    std::ofstream file(file_path, std::ios::binary);
    if (file) {
        file.write(reinterpret_cast<const char*>(data), size);
    }
}

bool createDirectoryIfNotExists(const std::string& path) {
    std::filesystem::path dirPath(path);

    // 检查路径是否存在
    if (!std::filesystem::exists(dirPath)) {
        try {
            if (std::filesystem::create_directories(dirPath)) {
                std::cout << "M1k::INFO~~~~~~~~Directory created: " << dirPath << std::endl;
                return true;
            } else {
                std::cerr << "M1k::INFO~~~~~~~~Failed to create directory: " << dirPath << std::endl;
                return false;
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << e.what() << std::endl;
            return false;
        }
    } else {
        return true; // 目录已存在
    }
}


void* allocateAligned(size_t size, size_t alignment) {
    void* ptr = nullptr;
    // 分配原始内存，额外空间用于存储原始指针和确保足够的对齐空间
    // 使用alignment - 1 + sizeof(void*)来确保有足够的空间进行对齐调整和存储原始指针
    void* original = new char[size + alignment - 1 + sizeof(void*)];
    if (original) {
        ptr = static_cast<char*>(original) + sizeof(void*);
        ptr = reinterpret_cast<void*>((reinterpret_cast<size_t>(ptr) + alignment - 1) & ~(alignment - 1));
        *(static_cast<void**>(ptr) - 1) = original;
    }
    return ptr;
}

void deallocateAligned(void* aligned_ptr) {
    if (aligned_ptr) {
        // 获取原始指针并释放内存
        void* original = *(static_cast<void**>(aligned_ptr) - 1);
        delete[] static_cast<char*>(original);
    }
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


