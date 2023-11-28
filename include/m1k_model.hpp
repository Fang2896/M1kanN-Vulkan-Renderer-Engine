//
// Created by fangl on 2023/11/22.
//

#pragma once

#include "m1k_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


namespace m1k {

class M1kModel {
   public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };

    struct Builder {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
    };


    M1kModel(M1kDevice& device, const M1kModel::Builder &builder);
    ~M1kModel();

    M1kModel(const M1kModel&) = delete;
    M1kModel operator=(const M1kModel&) = delete;

    void bind(VkCommandBuffer command_buffer);
    void draw(VkCommandBuffer command_buffer);

   private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);
    void createIndexBuffers(const std::vector<uint32_t> &indices);

    M1kDevice& m1K_device_;

    VkBuffer vertex_buffer_;
    VkDeviceMemory vertex_buffer_memory_;
    uint32_t vertex_count_;

    bool has_index_buffer_{false};
    VkBuffer index_buffer_;
    VkDeviceMemory index_buffer_memory_;
    uint32_t index_count_;
};

}
