//
// Created by fangl on 2024/02/26.
//

#pragma once

#include "core/m1k_device.hpp"
#include "core/m1k_pipeline.hpp"
#include "m1k_frame_info.hpp"
#include "objects/m1k_game_object.hpp"
#include "ui/m1k_camera.hpp"

// std
#include <memory>

namespace m1k {

class PbrRenderSystem {
   public:
    PbrRenderSystem(M1kDevice &device,
                    VkRenderPass render_pass,
                    VkDescriptorSetLayout global_set_layout,
                    VkDescriptorSetLayout pbr_set_layout);
    ~PbrRenderSystem();

    // copy version delete
    PbrRenderSystem(const PbrRenderSystem&) = delete;
    PbrRenderSystem &operator=(const PbrRenderSystem&) = delete;

    void render(FrameInfo &frame_info);

   private:
    void createPipelineLayout(VkDescriptorSetLayout global_set_layout,
                              VkDescriptorSetLayout pbr_set_layout);
    void createPipeline(VkRenderPass render_pass);

    M1kDevice &m1k_device_;
    std::unique_ptr<M1kPipeline> m1k_pipeline_;
    VkPipelineLayout pipeline_layout_;
};

}
