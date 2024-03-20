//
// Created by fangl on 2024/3/2.
//

#include "m1k_mesh.hpp"




namespace m1k {

M1kMesh::M1kMesh(M1kDevice& device,
                 M1kDescriptorSetLayout &set_layout,
                 M1kDescriptorPool &pool,
                 std::vector<M1kVertex>& vertices,
                 std::vector<uint32_t>& indices, M1kMaterialSet material_set, uint32_t flags)
    : m1k_device_(device), vertices_(vertices), indices_(indices),
      material_set_(material_set), flags_(flags)
{
      createVertexBuffers(vertices_);
      createIndexBuffers(indices_);
      createDescriptorSets(set_layout, pool);
}

std::vector<VkVertexInputBindingDescription> M1kMesh::getBindingDescriptions() {
    return {{0, sizeof(M1kVertex), VK_VERTEX_INPUT_RATE_VERTEX}};
}

std::vector<VkVertexInputAttributeDescription> M1kMesh::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

    attribute_descriptions.push_back({0,0,VK_FORMAT_R32G32B32_SFLOAT,static_cast<uint32_t>(offsetof(M1kVertex, position))});
    attribute_descriptions.push_back({1,0,VK_FORMAT_R32G32B32A32_SFLOAT,static_cast<uint32_t>(offsetof(M1kVertex, normal))});
    attribute_descriptions.push_back({2,0,VK_FORMAT_R32G32B32_SFLOAT,static_cast<uint32_t>(offsetof(M1kVertex, tangent))});
    attribute_descriptions.push_back({3,0,VK_FORMAT_R32G32_SFLOAT,static_cast<uint32_t>(offsetof(M1kVertex, uv))});

    return attribute_descriptions;
}

void M1kMesh::createVertexBuffers(const std::vector<M1kVertex> &vertices) {
    vertex_count_ = static_cast<uint32_t>(vertices.size());
    assert(vertex_count_ >= 3 && "M1kVertex count must be at least 3");
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertex_count_;
    uint32_t vertex_size = sizeof(vertices[0]);

    M1kBuffer staging_buffer{
        m1k_device_,
        vertex_size,
        vertex_count_,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    staging_buffer.map();
    staging_buffer.writeToBuffer((void *)vertices.data());

    vertex_buffer_ = std::make_unique<M1kBuffer>(
        m1k_device_,
        vertex_size,
        vertex_count_,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m1k_device_.copyBuffer(staging_buffer.getBuffer(), vertex_buffer_->getBuffer(), buffer_size);
}

void M1kMesh::createIndexBuffers(const std::vector<uint32_t> &indices) {
    index_count_ = static_cast<uint32_t>(indices.size());
    has_index_buffer_ = index_count_ > 0;
    if(!has_index_buffer_) {
        return;
    }

    assert(index_count_ >= 3 && "Index count must be at least 3");
    VkDeviceSize buffer_size = sizeof(indices[0]) * index_count_;
    uint32_t index_size = sizeof(indices[0]);

    M1kBuffer staging_buffer {
        m1k_device_,
        index_size,
        index_count_,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    staging_buffer.map();
    staging_buffer.writeToBuffer((void*)indices.data());

    // staging buffer_ is good for static data
    index_buffer_ = std::make_unique<M1kBuffer>(
        m1k_device_,
        index_size,
        index_count_,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m1k_device_.copyBuffer(staging_buffer.getBuffer(), index_buffer_->getBuffer(), buffer_size);
}

void M1kMesh::createDescriptorSets(M1kDescriptorSetLayout &set_layout, M1kDescriptorPool &pool) {
    material_ubo_buffer_ = std::make_unique<M1kBuffer>(m1k_device_,
                                                sizeof(MaterialUbo), 1,
                                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    material_ubo_buffer_->map();

    // update material UBO
    MaterialUbo material_ubo;

    material_ubo.model = glm::mat4(1.0f);
    material_ubo.model_inv = glm::mat4(1.0f);

    material_ubo.base_color_factor = material_set_.base_color_factor;
    material_ubo.emissive_factor = material_set_.emissive_factor;
    material_ubo.metallic_factor = material_set_.metallic_factor;
    material_ubo.roughness_factor = material_set_.roughness_factor;
    material_ubo.occlusion_factor = material_set_.occlusion_factor;
    material_ubo.flags = flags_;

    material_ubo_buffer_->writeToBuffer(&material_ubo);
    material_ubo_buffer_->flush();

    auto material_buffer_info = material_ubo_buffer_->descriptorInfo();
    auto writer = M1kDescriptorWriter(set_layout, pool)
        .writeBuffer(0, &material_buffer_info);

    if(material_set_.base_color_texture != nullptr) {
        writer.writeImage(1, &material_set_.base_color_texture->getDescriptorImageInfo());
    }
    if(material_set_.normal_texture != nullptr) {
        writer.writeImage(2, &material_set_.normal_texture->getDescriptorImageInfo());
    }
    if(material_set_.roughness_metalness_texture != nullptr) {
        writer.writeImage(3, &material_set_.roughness_metalness_texture->getDescriptorImageInfo());
    }
    if(material_set_.occlusion_texture != nullptr) {
        writer.writeImage(4, &material_set_.occlusion_texture->getDescriptorImageInfo());
    }
    if(material_set_.emissive_texture != nullptr) {
        writer.writeImage(5, &material_set_.emissive_texture->getDescriptorImageInfo());
    }


    writer.build(mesh_descriptor_set_);
}

void M1kMesh::bind(VkCommandBuffer command_buffer, VkPipelineLayout& pipeline_layout) {
    vkCmdBindDescriptorSets(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout,
        1, 1,
        &mesh_descriptor_set_,
        0, nullptr);

    VkBuffer buffers[] = {vertex_buffer_->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);

    if(has_index_buffer_) {
        vkCmdBindIndexBuffer(command_buffer, index_buffer_->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

void M1kMesh::draw(VkCommandBuffer command_buffer) {
    if(has_index_buffer_) {
        vkCmdDrawIndexed(command_buffer, index_count_, 1, 0, 0, 0);
    } else {
        vkCmdDraw(command_buffer, vertex_count_, 1, 0, 0);
    }
}




}


