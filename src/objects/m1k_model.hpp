//
// Created by fangl on 2023/11/22.
//

#pragma once

#include "m1k_mesh.hpp"
#include "m1k_buffer.hpp"
#include "m1k_device.hpp"
#include "m1k_data_struct.hpp"

// libs
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <iostream>
#include <unordered_map>

#include <vector>
#include <memory>


namespace m1k {

class M1kModel {
   public:
    M1kModel(M1kDevice& device,
             M1kDescriptorSetLayout &set_layout,
             M1kDescriptorPool &pool,
             const std::string& filepath);
    ~M1kModel();

    M1kModel(const M1kModel&) = delete;
    M1kModel operator=(const M1kModel&) = delete;

    void draw(VkCommandBuffer command_buffer, VkPipelineLayout& pipeline_layout);

   private:
    void loadModelFromGLTF(const std::string& filepath);

    M1kDevice& m1K_device_;
    M1kDescriptorSetLayout &descriptor_set_layout_;
    M1kDescriptorPool &descriptor_pool_;

    std::unique_ptr<M1kBuffer> material_ubo_buffer_;

    std::vector<std::unique_ptr<M1kMesh>> meshes_{};

    std::string model_directory_path_{};
    std::unordered_map<std::string, std::shared_ptr<M1kTexture>> textures_{};
};

}
