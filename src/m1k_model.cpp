//
// Created by fangl on 2023/11/22.
//

#include "m1k_model.hpp"
#include "m1k_utils.hpp"

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <iostream>
#include <cstring>
#include <unordered_map>

namespace std {
template<>
struct hash<m1k::M1kModel::Vertex> {
    size_t operator()(m1k::M1kModel::Vertex const &vertex) const {
        size_t seed = 0;
        m1k::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
        return seed;
    }
};
}

namespace m1k {

M1kModel::M1kModel(M1kDevice& device, const M1kModel::Builder &builder) : m1K_device_(device) {
    createVertexBuffers(builder.vertices);
    createIndexBuffers(builder.indices);
}

M1kModel::~M1kModel() = default;

std::unique_ptr<M1kModel> M1kModel::createModelFromFile(M1kDevice &device, const std::string &filepath) {
    Builder builder{};
    builder.loadModel(filepath);

    std::cout << "Vertex Count : " << builder.vertices.size() << "\n";

    return std::make_unique<M1kModel>(device, builder);
}

void M1kModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
    vertex_count_ = static_cast<uint32_t>(vertices.size());
    assert(vertex_count_ >= 3 && "Vertex count must be at least 3");
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;
    uint32_t vertex_size = sizeof(vertices[0]);

    M1kBuffer staging_buffer{
        m1K_device_,
        vertex_size,
        vertex_count_,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    staging_buffer.map();
    staging_buffer.writeToBuffer((void *)vertices.data());

    vertex_buffer_ = std::make_unique<M1kBuffer>(
        m1K_device_,
        vertex_size,
        vertex_count_,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m1K_device_.copyBuffer(staging_buffer.getBuffer(), vertex_buffer_->getBuffer(), buffer_size);
}

void M1kModel::createIndexBuffers(const std::vector<uint32_t> &indices) {
    index_count_ = static_cast<uint32_t>(indices.size());
    has_index_buffer_ = index_count_ > 0;
    if(!has_index_buffer_) {
        return;
    }

    assert(index_count_ >= 3 && "Index count must be at least 3");
    VkDeviceSize buffer_size = sizeof(indices[0]) * index_count_;
    uint32_t index_size = sizeof(indices[0]);

    M1kBuffer staging_buffer {
        m1K_device_,
        index_size,
        index_count_,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    staging_buffer.map();
    staging_buffer.writeToBuffer((void*)indices.data());

    // staging buffer_ is good for static data
    index_buffer_ = std::make_unique<M1kBuffer>(
        m1K_device_,
        index_size,
        index_count_,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m1K_device_.copyBuffer(staging_buffer.getBuffer(), index_buffer_->getBuffer(), buffer_size);
}

void M1kModel::bind(VkCommandBuffer command_buffer) {
    VkBuffer buffers[] = {vertex_buffer_->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

    if(has_index_buffer_) {
        vkCmdBindIndexBuffer(command_buffer, index_buffer_->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
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

    std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

    attribute_descriptions.push_back({0,0,VK_FORMAT_R32G32B32_SFLOAT,static_cast<uint32_t>(offsetof(Vertex, position))});
    attribute_descriptions.push_back({1,0,VK_FORMAT_R32G32B32_SFLOAT,static_cast<uint32_t>(offsetof(Vertex, color))});
    attribute_descriptions.push_back({2,0,VK_FORMAT_R32G32B32_SFLOAT,static_cast<uint32_t>(offsetof(Vertex, normal))});
    attribute_descriptions.push_back({3,0,VK_FORMAT_R32G32_SFLOAT,static_cast<uint32_t>(offsetof(Vertex, uv))});

    return attribute_descriptions;
}

void M1kModel::Builder::loadModel(const std::string &filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> unique_vertices{};
    for(const auto& shape : shapes) {
        for(const auto& index : shape.mesh.indices) {
            Vertex vertex{};
            if(index.vertex_index >= 0) {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2]
                };
            }

            if(index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            }

            if(index.texcoord_index >= 0) {
                vertex.uv = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };
            }

            // eliminate duplicates
            if(unique_vertices.count(vertex) == 0) {
                unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(unique_vertices[vertex]);
        }
    }

}

}



