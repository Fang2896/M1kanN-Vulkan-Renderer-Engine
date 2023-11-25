//
// Created by fangl on 2023/11/25.
//

#pragma once

#include "m1k_model.hpp"

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
    Transform2dComponent transform_2d_component{};

   private:
    M1kGameObject(id_t objectId) : id(objectId) {}

    id_t id;
};


}

