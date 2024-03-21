//
// Created by fangl on 2023/11/30.
//

#include "m1k_game_object.hpp"

namespace m1k {

M1kGameObject M1kGameObject::makePointLight(float intensity, float radius, glm::vec3 color) {
    M1kGameObject point_light_object = M1kGameObject::createGameObject(GameObjectType::PointLight);
    point_light_object.color = color;
    point_light_object.transform.scale.x = radius;
    point_light_object.point_light = std::make_unique<PointLightComponent>();
    point_light_object.point_light->light_intensity = intensity;

    return point_light_object;
}

}
