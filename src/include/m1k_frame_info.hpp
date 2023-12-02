//
// Created by fangl on 2023/12/1.
//

#pragma once

#include "m1k_camera.hpp"
#include "m1k_game_object.hpp"

// lib
#include <vulkan/vulkan.h>

namespace m1k {

#define MAX_LIGHTS 10

struct PointLight {
    glm::vec4 position{};   // ignore w
    glm::vec4 color{};  // w is intensity
};

// automatically align in Vulkan
struct GlobalUbo {
    glm::mat4 projection_matrix{1.0f};
    glm::mat4 view_matrix{1.0f};
    glm::mat4 inverse_view_matrix{1.0f};    // last column is the camera position
    glm::vec4 ambient_light_color{1.0f, 1.0f, 1.0f, 0.02f};
    PointLight point_lights[MAX_LIGHTS];
    int num_lights;
};

struct FrameInfo {
    int frame_index;
    float frame_time;
    VkCommandBuffer command_buffer;
    M1kCamera &camera;
    VkDescriptorSet global_descriptor_set;
    M1kGameObject::Map &game_objects;
};


}
