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
struct alignas( 16 ) GlobalUbo {
    glm::mat4 projection_matrix{1.0f};
    glm::mat4 view_matrix{1.0f};
    glm::mat4 inverse_view_matrix{1.0f};    // last column is the camera position
    glm::vec4 ambient_light_color{1.0f, 1.0f, 1.0f, 0.02f}; // rgb,intensity
    glm::vec4 direct_light{1.0f, -1.0f, 1.0f, 0.5f};  // x,y,z,intensity
    PointLight point_lights[MAX_LIGHTS];
    uint32_t num_lights;
};

struct alignas( 16 ) MaterialUbo {
    glm::vec4 base_color_factor;
    glm::mat4 model;
    glm::mat4 model_inv;

    glm::vec3 emissive_factor;
    float  metallic_factor;

    float   roughness_factor;
    float   occlusion_factor;
    uint32_t   flags;
};

struct MeshDraw {
    std::unique_ptr<M1kBuffer> position_buffer;
    std::unique_ptr<M1kBuffer> index_buffer;
    std::unique_ptr<M1kBuffer> tangent_buffer;
    std::unique_ptr<M1kBuffer> normal_buffer;
    std::unique_ptr<M1kBuffer> texcoord_buffer;

    std::unique_ptr<M1kBuffer> material_buffer;
    MaterialUbo material_ubo;
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
