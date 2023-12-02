//
// Created by fangl on 2023/11/25.
//

#pragma once

#include "m1k_model.hpp"

// libs
#include <glm/ext/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

// might be re-implemented by ECS strategy

namespace m1k {

struct TransformComponent {
    glm::vec3 translation{};    // position offset
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 rotation{};

    // faster mat4 transform
    glm::mat4 mat4();
    glm::mat3 normalMatrix();

};

class M1kGameObject {
   public:
    using id_t = unsigned int;
    using Map = std::unordered_map<id_t, M1kGameObject>;

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

