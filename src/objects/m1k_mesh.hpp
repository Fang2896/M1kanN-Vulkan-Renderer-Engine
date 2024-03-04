//
// Created by fangl on 2024/3/2.
//

#pragma once

#include "m1k_buffer.hpp"
#include "m1k_texture.hpp"
#include "m1k_utils.hpp"
#include "m1k_data_struct.hpp"
#include "m1k_descriptor.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
#include <string>
#include <utility>
#include <vector>


namespace m1k {

struct M1kVertex {
    glm::vec3 position{0.0f,0.0f,0.0f};
//    glm::vec3 color{1.0f,1.0f,1.0f};
    glm::vec3 normal{0.0f,0.0f,0.0f};
    glm::vec4 tangent{0.0f,0.0f,0.0f,0.0f};
    glm::vec2 uv{0.0f,0.0f};

    bool operator==(const M1kVertex& other) const {
        return position == other.position &&
//               color == other.color &&
               normal == other.normal &&
               tangent == other.tangent &&
               uv == other.uv;
    }
};


// for each mesh
struct M1kMaterialSet {
    std::shared_ptr<M1kTexture> base_color_texture{};            // bd = 1
    std::shared_ptr<M1kTexture> roughness_metalness_texture{};  // bd = 2
    std::shared_ptr<M1kTexture> occlusion_texture{};            // bd = 3
    std::shared_ptr<M1kTexture> emissive_texture{};             // bd = 4
    std::shared_ptr<M1kTexture> normal_texture{};               // bd = 5
//    std::shared_ptr<M1kTexture> null_texture{};

    glm::vec4 base_color_factor{1.0f};
    glm::vec3 emissive_factor{1.0f};
    float metallic_factor{1.0f};
    float roughness_factor{1.0f};
    float occlusion_factor{1.0f};

//    M1kMaterialSet(std::shared_ptr<M1kTexture> base_color,
//                std::shared_ptr<M1kTexture> roughness_metalness,
//                std::shared_ptr<M1kTexture> occlusion,
//                std::shared_ptr<M1kTexture> emissive,
//                std::shared_ptr<M1kTexture> normal)
//        : base_color_texture(std::move(base_color)),
//          roughness_metalness_texture(std::move(roughness_metalness)),
//          occlusion_texture(std::move(occlusion)),
//          emissive_texture(std::move(emissive)),
//          normal_texture(std::move(normal))  {}

    M1kMaterialSet() = default;
    ~M1kMaterialSet() = default;

//    uint32_t getTextureFlags() {
//        uint32_t texture_flags = 0;
//        if (base_color_texture)              texture_flags |= 1 << 0;
//        if (normal_texture)                 texture_flags |= 1 << 1;
//        if (roughness_metalness_texture)    texture_flags |= 1 << 2;
//        if (occlusion_texture)              texture_flags |= 1 << 3;
//        if (emissive_texture)               texture_flags |= 1 << 4;
//
//        return texture_flags;
//    }
};


class M1kMesh {
   public:
    M1kMesh(M1kDevice& device,
            M1kDescriptorSetLayout &set_layout,
            M1kDescriptorPool &pool,
            std::vector<M1kVertex>& vertices,
            std::vector<uint32_t>& indices, M1kMaterialSet material_set, uint32_t flag);

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

    void bind(VkCommandBuffer command_buffer, VkPipelineLayout& pipeline_layout);
    void draw(VkCommandBuffer command_buffer);

   private:
    void createVertexBuffers(const std::vector<M1kVertex> &vertices);
    void createIndexBuffers(const std::vector<uint32_t> &indices);
    void createDescriptorSets(M1kDescriptorSetLayout &set_layout, M1kDescriptorPool &pool);

    M1kDevice& m1k_device_;
    VkDescriptorSet mesh_descriptor_set_;
    std::unique_ptr<M1kBuffer> material_ubo_buffer_;

    std::vector<M1kVertex> vertices_{};
    std::vector<uint32_t> indices_{};
    M1kMaterialSet material_set_;
    uint32_t flags_ = 0;

    std::unique_ptr<M1kBuffer> vertex_buffer_;
    uint32_t vertex_count_;

    bool has_index_buffer_{false};
    std::unique_ptr<M1kBuffer> index_buffer_;
    uint32_t index_count_;
};


}
