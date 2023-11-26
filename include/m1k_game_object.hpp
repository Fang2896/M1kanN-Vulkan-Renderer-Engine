//
// Created by fangl on 2023/11/25.
//

#pragma once

#include "m1k_model.hpp"

// libs
#include <glm/ext/matrix_transform.hpp>

// std
#include <memory>

namespace m1k {

struct Transform2dComponent {
    glm::vec2 translation{};    // position offset
    glm::vec2 scale{1.0f, 1.0f};
    float rotation;

    glm::mat2 mat2() {
        const float s = glm::sin(rotation);
        const float c = glm::cos(rotation);
        glm::mat2 rotMat({c, s}, {-s, c});

        glm::mat2 scaleMat{{scale.x, 0.0f}, {0.0f, scale.y}};

        return rotMat * scaleMat;
    }
};

struct TransformComponent {
    glm::vec3 translation{};    // position offset
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 rotation{};

//    glm::mat4 mat4() {
//        auto transform = glm::translate(glm::mat4{1.0f}, translation);
//        transform = glm::rotate(transform, rotation.y, {0.0f, 1.0f, 0.0f});
//        transform = glm::rotate(transform, rotation.x, {1.0f, 0.0f, 0.0f});
//        transform = glm::rotate(transform, rotation.z, {0.0f, 1.0f, 1.0f});
//        transform = glm::scale(transform, scale);
//        return transform;
//    }

    // faster mat4 transform
    glm::mat4 mat4() {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
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

};

class M1kGameObject {
   public:
    using id_t = unsigned int;

    static M1kGameObject createGameObject() {
        static id_t currentId = 0;
        return M1kGameObject{currentId++};
    }

    M1kGameObject(const M1kGameObject&) = delete;
    M1kGameObject &operator=(const M1kGameObject&) = delete;
    M1kGameObject(M1kGameObject &&) = default;
    M1kGameObject &operator=(M1kGameObject &&) = default;

    id_t getId() { return id; }

    std::shared_ptr<M1kModel> model{};
    glm::vec3 color{};
    TransformComponent transform{};

   private:
    M1kGameObject(id_t objectId) : id(objectId) {}

    id_t id;
};


}

