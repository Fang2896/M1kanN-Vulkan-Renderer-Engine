//
// Created by fangl on 2023/11/22.
//

#include "m1k_model.hpp"

// std
#include <cassert>


namespace m1k {

M1kModel::M1kModel(M1kDevice& device, const M1kModel::Builder &builder) : m1K_device_(device) {
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
}

M1kModel::~M1kModel() {
    vkDestroyBuffer(m1K_device_.device(), vertex_buffer_, nullptr);
    vkFreeMemory(m1K_device_.device(), vertex_buffer_memory_, nullptr);

    if(has_index_buffer_) {
        vkDestroyBuffer(m1K_device_.device(), index_buffer_, nullptr);
        vkFreeMemory(m1K_device_.device(), index_buffer_memory_, nullptr);
    }
}

void M1kModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
    vertex_count_ = static_cast<uint32_t>(vertices.size());
    assert(vertex_count_ >= 3 && "Vertex count must be at least 3");
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;

    // staging buffer is good for static data
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    m1K_device_.createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer,
        staging_buffer_memory);

    void *data;
    vkMapMemory(m1K_device_.device(), staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, vertices.data(), static_cast<uint32_t>(buffer_size));
    vkUnmapMemory(m1K_device_.device(), staging_buffer_memory);

    m1K_device_.createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertex_buffer_,
        vertex_buffer_memory_);

    m1K_device_.copyBuffer(staging_buffer, vertex_buffer_, buffer_size);

    vkDestroyBuffer(m1K_device_.device(), staging_buffer, nullptr);
    vkFreeMemory(m1K_device_.device(), staging_buffer_memory, nullptr);
}

void M1kModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
    index_count_ = static_cast<uint32_t>(indices.size());
    has_index_buffer_ = index_count_ > 0;
    if(!has_index_buffer_) {
        return;
    }

    assert(index_count_ >= 3 && "Index count must be at least 3");
    VkDeviceSize buffer_size = sizeof(indices[0]) * index_count_;

    // staging buffer is good for static data
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    m1K_device_.createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer,
        staging_buffer_memory);

    void *data;
    vkMapMemory(m1K_device_.device(), staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(buffer_size));
    vkUnmapMemory(m1K_device_.device(), staging_buffer_memory);

    m1K_device_.createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        index_buffer_,
        index_buffer_memory_);

    m1K_device_.copyBuffer(staging_buffer, index_buffer_, buffer_size);

    vkDestroyBuffer(m1K_device_.device(), staging_buffer, nullptr);
    vkFreeMemory(m1K_device_.device(), staging_buffer_memory, nullptr);
}

void M1kModel::bind(VkCommandBuffer command_buffer) {
    VkBuffer buffers[] = {vertex_buffer_};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

    if(has_index_buffer_) {
        vkCmdBindIndexBuffer(command_buffer, index_buffer_, 0, VK_INDEX_TYPE_UINT32);
    }
}

void M1kModel::draw(VkCommandBuffer command_buffer) {
    if(has_index_buffer_) {
        vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
    } else {
        vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
    }
}

std::vector<VkVertexInputBindingDescription> M1kModel::Vertex::getBindingDescriptions() {
    return {{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};
}

std::vector<VkVertexInputAttributeDescription> M1kModel::Vertex::getAttributeDescriptions() {
//    return
//    {
//        {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, position)},
//        {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)}
//    };

    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions;
}

}



