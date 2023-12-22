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

struct PointLightComponent {
    float light_intensity = 1.0f;
};

class M1kGameObject {
   public:
    using id_t = unsigned int;
    using Map = std::unordered_map<id_t, M1kGameObject>;

    static M1kGameObject createGameObject() {
        static id_t currentId = 0;
        return M1kGameObject{currentId++};
    }

    static M1kGameObject makePointLight(float intensity = 10.f,
                                        float radius = 0.1f,
                                        glm::vec3 color = glm::vec3(1.f));

    M1kGameObject(const M1kGameObject&) = delete;
    M1kGameObject &operator=(const M1kGameObject&) = delete;
    M1kGameObject(M1kGameObject &&) = default;
    M1kGameObject &operator=(M1kGameObject &&) = default;

    id_t getId() { return id_; }

    glm::vec3 color{};
    TransformComponent transform{};

    // optional pointer components
    std::shared_ptr<M1kModel> model{};

    std::unique_ptr<PointLightComponent> point_light = nullptr;

   private:
    M1kGameObject(id_t objectId) : id_(objectId) {}

    id_t id_;
};


}

