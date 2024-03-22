//
// Created by fangl on 2024/02/26.
//

#pragma once

#include "core/m1k_device.hpp"
#include "core/m1k_pipeline.hpp"
#include "m1k_frame_info.hpp"
#include "objects/m1k_game_object.hpp"
#include "ui/m1k_camera.hpp"
#include "m1k_config.hpp"

// std
#include <memory>

namespace m1k {

class BindlessPbrRenderSystem {
   public:
    BindlessPbrRenderSystem(M1kDevice &device,
                    VkRenderPass render_pass,
                    VkDescriptorSetLayout global_set_layout,
                    VkDescriptorSetLayout pbr_set_layout,
                    VkDescriptorSetLayout bindless_set_layout);
    ~BindlessPbrRenderSystem();

    // copy version delete
    BindlessPbrRenderSystem(const BindlessPbrRenderSystem&) = delete;
    BindlessPbrRenderSystem &operator=(const BindlessPbrRenderSystem&) = delete;

    void render(FrameInfo &frame_info);
    void updateBindlessTextures(FrameInfo &frame_info);

   private:
    void createPipelineLayout();
    void createPipeline(VkRenderPass render_pass);

    M1kDevice &m1k_device_;
    VkDescriptorSetLayout global_set_layout_;
    VkDescriptorSetLayout pbr_set_layout_;
    VkDescriptorSetLayout bindless_set_layout_;
    std::unique_ptr<M1kPipeline> m1k_pipeline_;
    VkPipelineLayout pipeline_layout_;
};

}
