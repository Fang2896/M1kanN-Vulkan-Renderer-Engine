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
        glm::vec2 position;
        glm::vec3 color;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };


    M1kModel(M1kDevice& device, const std::vector<Vertex>& vertices);
    ~M1kModel();

    M1kModel(const M1kModel&) = delete;
    M1kModel operator=(const M1kModel&) = delete;

    void bind(VkCommandBuffer command_buffer);
    void draw(VkCommandBuffer command_buffer);

   private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);

    M1kDevice& m1K_device_;
    VkBuffer vertex_buffer_;
    VkDeviceMemory vertex_buffer_memory_;
    uint32_t vertex_count_;
};

}
