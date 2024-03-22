//
// Created by fangl on 2023/11/22.
//

#include "m1k_model.hpp"

// Define these only in *one* .cc file.
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf.h"


namespace std {
template<>
struct hash<m1k::M1kVertex> {
    size_t operator()(m1k::M1kVertex const &vertex) const {
        size_t seed = 0;
        m1k::hashCombine(seed, vertex.position,  vertex.normal, vertex.uv);
        return seed;
    }
};
}

namespace m1k {

M1kModel::M1kModel(M1kDevice& device,
                   M1kDescriptorSetLayout &set_layout,
                   M1kDescriptorPool &pool,
                   const std::string& filepath)
    : m1K_device_(device), descriptor_set_layout_(set_layout), descriptor_pool_(pool)
{
    std::string default_texture_path = "../assets/textures/dummy_texture.png";
    dummy_texture_ = std::make_shared<M1kTexture>(m1K_device_,
                                                default_texture_path);
    to_update_textures_["dummy_texture"] = dummy_texture_;

    loadModelFromGLTF(filepath);
}

M1kModel::~M1kModel() = default;

void M1kModel::draw(VkCommandBuffer command_buffer,
                    VkDescriptorSet bindless_set,
                    VkPipelineLayout& pipeline_layout) {
    // bind bindless descriptor set
    vkCmdBindDescriptorSets(
        command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout,
        1, 1,
        &bindless_set,
        0, nullptr);

    for(auto& mesh : meshes_) {
        mesh->bind(command_buffer, pipeline_layout);
        mesh->draw(command_buffer);
    }
}



void M1kModel::loadModelFromGLTF(const std::string& filepath) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(
        &model, &err, &warn, filepath);

    if (!warn.empty()) std::cout << "Warn: " << warn << std::endl;

    if (!err.empty()) std::cerr << "Err: " << err << std::endl;

    if (!ret) {
        std::cout << "M1k::WARN========Failed to parse glTF" << std::endl;
        return;
    }

    std::cout << "M1k::INFO~~~~~~~~Loaded glTF model: " << filepath << std::endl;
    model_directory_path_ = filepath.substr(0, filepath.find_last_of("/\\"));

    // First load all textures and record new to be updated texture
    for(const auto& img : model.images) {
        if(textures_.find(img.uri) == textures_.end()) {
            textures_[img.uri] =
                std::make_shared<M1kTexture>(m1K_device_,
                                             model_directory_path_ + "/" + img.uri);
            to_update_textures_[img.uri] = textures_[img.uri];  // for update
        }
    }

    for (const auto& node : model.nodes) {
        if(node.mesh < 0 || node.mesh >= model.meshes.size()) continue;

        const auto& mesh = model.meshes[node.mesh];
        std::cout << "M1k::INFO~~~~~~~~Mesh name: " << mesh.name << std::endl;

        TransformComponent transform_component;
        const auto& node_scale = node.scale;
        const auto& node_rotation = node.rotation;
        const auto& node_translation = node.translation;

        if(node_scale.size() != 0) {
            transform_component.scale = {node_scale[0], node_scale[1], node_scale[2]};
        }
        if(node_rotation.size() != 0) {
            transform_component.rotation = {node_rotation[0], node_rotation[1], node_rotation[2]};
        }
        if(node_translation.size() != 0) {
            transform_component.translation = {node_translation[0], node_translation[1], node_translation[2]};
        }

        glm::mat4 node_transform = transform_component.mat4();
        glm::mat4 inv_transform = glm::inverse(node_transform);

        for(const auto& primitive : mesh.primitives) {
            const auto& attributes = primitive.attributes;

            std::vector<M1kVertex> vertices{};
            std::vector<uint32_t> indices{};
            M1kMaterialSet material_set;
            uint32_t flags = 0;

            material_set.transform = node_transform;
            material_set.inv_transform = inv_transform;

            // materials
            if (primitive.material >= 0) {
                const auto& material = model.materials[primitive.material];
                std::cout << "M1k::INFO~~~~~~~~Uses material: " << material.name << std::endl;

                if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                    int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
                    const auto& texture = model.textures[textureIndex];

                    int imageIndex = texture.source;
                    const auto& image = model.images[imageIndex];

                    if(textures_.find(image.uri) == textures_.end()) {
                        textures_[image.uri] =
                            std::make_shared<M1kTexture>(m1K_device_,
                                                         model_directory_path_ + "/" + image.uri);
                    }
                    material_set.base_color_texture_handle  = textures_[image.uri]->getIndex();

                    const auto& factor = material.pbrMetallicRoughness.baseColorFactor;
                    material_set.base_color_factor = glm::vec4(factor[0],factor[1],factor[2],factor[3]);
                    flags |= 1 << 0;

                    std::cout << "M1k::INFO~~~~~~~~Base color texture path: " << image.uri << std::endl;
                    std::cout << "M1k::INFO~~~~~~~~Base color factor: (" <<
                        factor[0] << "," << factor[1] << "," <<
                        factor[2] << "," << factor[3] << "," <<
                        ")" << std::endl;
                } else {
                    material_set.base_color_texture_handle = dummy_texture_->getIndex();
                }

                if (material.normalTexture.index >= 0) {
                    int textureIndex = material.normalTexture.index;
                    const auto& texture = model.textures[textureIndex];

                    int imageIndex = texture.source;
                    const auto& image = model.images[imageIndex];

                    if(textures_.find(image.uri) == textures_.end()) {
                        textures_[image.uri] =
                            std::make_shared<M1kTexture>(m1K_device_,
                                                         model_directory_path_ + "/" + image.uri);
                    }
                    material_set.normal_texture_handle  = textures_[image.uri]->getIndex();

                    const auto normal_scale = static_cast<float>(material.normalTexture.scale);
                    material_set.normal_scale = normal_scale;

                    flags |= 1 << 1;

                    std::cout << "M1k::INFO~~~~~~~~Normal texture path: " << image.uri << std::endl;
                    std::cout << "M1k::INFO~~~~~~~~Normal scale: " << normal_scale << std::endl;
                } else {
                    material_set.normal_texture_handle = dummy_texture_->getIndex();
                }

                if (material.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0) {
                    int textureIndex = material.pbrMetallicRoughness.metallicRoughnessTexture.index;
                    const auto& texture = model.textures[textureIndex];

                    int imageIndex = texture.source;
                    const auto& image = model.images[imageIndex];

                    if(textures_.find(image.uri) == textures_.end()) {
                        textures_[image.uri] =
                            std::make_shared<M1kTexture>(m1K_device_,
                                                         model_directory_path_ + "/" + image.uri);
                    }
                    material_set.roughness_metalness_texture_handle  = textures_[image.uri]->getIndex();

                    const auto metallic_factor = static_cast<float>(material.pbrMetallicRoughness.metallicFactor);
                    const auto roughness_factor = static_cast<float>(material.pbrMetallicRoughness.roughnessFactor);
                    material_set.metallic_factor = metallic_factor;
                    material_set.roughness_factor = roughness_factor;
                    flags |= 1 << 2;

                    std::cout << "M1k::INFO~~~~~~~~Metallic Roughness texture path: " << image.uri << std::endl;
                    std::cout << "M1k::INFO~~~~~~~~Metallic factor: " << metallic_factor << std::endl;
                    std::cout << "M1k::INFO~~~~~~~~Roughness factor: " << roughness_factor << std::endl;
                } else {
                    material_set.roughness_metalness_texture_handle = dummy_texture_->getIndex();
                }

                if (material.occlusionTexture.index >= 0) {
                    int textureIndex = material.occlusionTexture.index;
                    const auto& texture = model.textures[textureIndex];

                    int imageIndex = texture.source;
                    const auto& image = model.images[imageIndex];

                    if(textures_.find(image.uri) == textures_.end()) {
                        textures_[image.uri] =
                            std::make_shared<M1kTexture>(m1K_device_,
                                                         model_directory_path_ + "/" + image.uri);
                    }
                    material_set.occlusion_texture_handle  = textures_[image.uri]->getIndex();

                    const auto occlusion_factor = static_cast<float>(material.occlusionTexture.strength);
                    material_set.occlusion_factor = occlusion_factor;
                    flags |= 1 << 3;

                    std::cout << "M1k::INFO~~~~~~~~Occlusion texture path: " << image.uri << std::endl;
                    std::cout << "M1k::INFO~~~~~~~~Occlusion strength: " << occlusion_factor << std::endl;
                } else {
                    material_set.occlusion_texture_handle  = dummy_texture_->getIndex();
                }

                if (material.emissiveTexture.index >= 0) {
                    int textureIndex = material.emissiveTexture.index;
                    const auto& texture = model.textures[textureIndex];

                    int imageIndex = texture.source;
                    const auto& image = model.images[imageIndex];

                    if(textures_.find(image.uri) == textures_.end()) {
                        textures_[image.uri] =
                            std::make_shared<M1kTexture>(m1K_device_,
                                                         model_directory_path_ + "/" + image.uri);
                    }
                    material_set.emissive_texture_handle  = textures_[image.uri]->getIndex();

                    const auto& factor = material.emissiveFactor;
                    material_set.emissive_factor = glm::vec3(factor[0],factor[1],factor[2]);
                    flags |= 1 << 4;

                    std::cout << "M1k::INFO~~~~~~~~Emissive texture path: " << image.uri << std::endl;
                    std::cout << "M1k::INFO~~~~~~~~Emissive factor: (" <<
                        factor[0] << "," << factor[1] << "," <<
                        factor[2] << "," <<
                        ")" << std::endl;
                } else {
                    material_set.emissive_texture_handle  = dummy_texture_->getIndex();
                }

                auto it = material.extensions.find("KHR_materials_clearcoat");
                if(it != material.extensions.end()) {
                    // TODO: 增加clearCoat texture等 shading

                    // 获取其他Clear Coat属性，如clearCoatRoughnessFactor, clearCoatNormalTexture, etc.
                    // ...
                }
            }

            if (attributes.find("POSITION") != attributes.end()) {
                const auto& accessor = model.accessors[attributes.at("POSITION")];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];

                const float* pos_data = reinterpret_cast<const float*>((&(buffer.data[bufferView.byteOffset + accessor.byteOffset])));

                for (size_t i = 0; i < accessor.count; ++i) {
                    M1kVertex vertex{};
                    vertex.position = glm::vec3(pos_data[i * 3 + 0], pos_data[i * 3 + 1], pos_data[i * 3 + 2]);
                    vertices.push_back(vertex);
                }
            }

            if (attributes.find("NORMAL") != attributes.end()) {
                const auto& accessor = model.accessors[attributes.at("NORMAL")];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const float* normal_data = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));

                for (size_t i = 0; i < accessor.count; ++i) {
                    vertices[i].normal = glm::vec3(normal_data[i * 3 + 0], normal_data[i * 3 + 1], normal_data[i * 3 + 2]);
                }
            }

            if (attributes.find("TANGENT") != attributes.end()) {
                const auto& accessor = model.accessors[attributes.at("TANGENT")];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const float* tangent_data = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));

                for (size_t i = 0; i < accessor.count; ++i) {
                    vertices[i].tangent = glm::vec4(tangent_data[i * 4 + 0], tangent_data[i * 4 + 1], tangent_data[i * 4 + 2], tangent_data[i * 4 + 3]);
                }

                flags |= 1 << 5;
            }

            if (attributes.find("TEXCOORD_0") != attributes.end()) {
                const auto& accessor = model.accessors[attributes.at("TEXCOORD_0")];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];
                const float* uv_data = reinterpret_cast<const float*>(&(buffer.data[bufferView.byteOffset + accessor.byteOffset]));

                for (size_t i = 0; i < accessor.count; ++i) {
                    vertices[i].uv = glm::vec2(uv_data[i * 2 + 0], uv_data[i * 2 + 1]);
                }

                flags |= 1 << 6;
            }

            if (primitive.indices > -1) {
                const auto& accessor = model.accessors[primitive.indices];
                const auto& bufferView = model.bufferViews[accessor.bufferView];
                const auto& buffer = model.buffers[bufferView.buffer];

                const unsigned char* index_data = &(buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                    for (size_t i = 0; i < accessor.count; ++i) {
                        indices.push_back(static_cast<uint32_t>(index_data[i]));
                    }
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const uint16_t* index_data_short = reinterpret_cast<const uint16_t*>(index_data);
                    for (size_t i = 0; i < accessor.count; ++i) {
                        indices.push_back(static_cast<uint32_t>(index_data_short[i]));
                    }
                } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const uint32_t* index_data_int = reinterpret_cast<const uint32_t*>(index_data);
                    for (size_t i = 0; i < accessor.count; ++i) {
                        indices.push_back(index_data_int[i]);
                    }
                }
            }

            meshes_.push_back(std::make_unique<M1kMesh>(m1K_device_,
                                                        descriptor_set_layout_,
                                                        descriptor_pool_,
                                                        vertices, indices,
                                                        material_set, flags));
        }
    }

}


}



