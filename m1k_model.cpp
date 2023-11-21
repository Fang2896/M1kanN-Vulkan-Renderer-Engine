//
// Created by fangl on 2023/11/22.
//

#include "m1k_model.hpp"

// std
#include <cassert>


namespace m1k {

M1kModel::M1kModel(M1kDevice& device, const std::vector<Vertex>& vertices) : m1K_device_(device) {
    createVertexBuffers(vertices);
}

M1kModel::~M1kModel() {
    vkDestroyBuffer(m1K_device_.device(), vertex_buffer_, nullptr);
    vkFreeMemory(m1K_device_.device(), vertex_buffer_memory_, nullptr);
}

void M1kModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
    vertex_count_ = static_cast<uint32_t>(vertices.size());
    assert(vertex_count_ >= 3 && "Vertex count must be at least 3");
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;

    m1K_device_.createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertex_buffer_,
        vertex_buffer_memory_);

    void *data;
    vkMapMemory(m1K_device_.device(), vertex_buffer_memory_, 0, buffer_size, 0, &data);
    memcpy(data, vertices.data(), static_cast<uint32_t>(buffer_size));
    vkUnmapMemory(m1K_device_.device(), vertex_buffer_memory_);
}

void M1kModel::bind(VkCommandBuffer command_buffer) {
    VkBuffer buffers[] = {vertex_buffer_};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
}

void M1kModel::draw(VkCommandBuffer command_buffer) {
    vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
}

std::vector<VkVertexInputBindingDescription> M1kModel::Vertex::getBindingDescriptions() {
    return {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};
}

std::vector<VkVertexInputAttributeDescription> M1kModel::Vertex::getAttributeDescriptions() {
    return {{0, 0, VK_FORMAT_R32G32_SFLOAT, 0}};
}

}



