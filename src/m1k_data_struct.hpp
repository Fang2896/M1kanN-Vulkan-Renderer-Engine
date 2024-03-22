//
// Created by fangl on 2024/3/3.
//

#pragma once

#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

namespace m1k {


#define MAX_LIGHTS 10

struct alignas( 16 ) PointLight {
    glm::vec4 position{};   // ignore w
    glm::vec4 color{};  // w is intensity
};

// automatically align in Vulkan
struct alignas( 16 ) GlobalUbo {
    glm::mat4 projection_matrix{1.0f};
    glm::mat4 view_matrix{1.0f};
    glm::mat4 inverse_view_matrix{1.0f};    // last column is the camera position
    glm::vec4 ambient_light_color{1.0f, 1.0f, 1.0f, 0.001f}; // rgb,intensity
    glm::vec4 direct_light{1.0f, -1.0f, 1.0f, 0.5f};  // x,y,z,intensity
    struct PointLight point_lights[MAX_LIGHTS];
    uint32_t num_lights;
};

struct alignas( 16 ) MaterialUbo {
    glm::mat4 model;
    glm::mat4 model_inv;

    // x: color,
    // y: normal,
    // z: emissive,
    // w: occlusion
    glm::uvec4 color_normal_emi_occ_texture_handles{0};

    // x: roughness,
    // y: metallic,
    // z: flags
    // w: ignore
    glm::uvec4 rough_meta_flags{0};

    // x: normal_scale
    // y: occlusion_factor
    // z: roughness_factor
    // w: metallic_factor
    glm::vec4 nor_occ_rough_meta_factor{1.0f};

    glm::vec3 emissive_factor{0.0f};
    glm::vec4 base_color_factor{1.0f};
};

//struct MeshDraw {
//    std::unique_ptr<M1kBuffer> position_buffer;
//    std::unique_ptr<M1kBuffer> index_buffer;
//    std::unique_ptr<M1kBuffer> tangent_buffer;
//    std::unique_ptr<M1kBuffer> normal_buffer;
//    std::unique_ptr<M1kBuffer> texcoord_buffer;
//
//    std::unique_ptr<M1kBuffer> material_buffer;
//    MaterialUbo material_ubo;
//};


}

