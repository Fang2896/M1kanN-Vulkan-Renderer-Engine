//
// Created by fangl on 2023/11/26.
//

#pragma once

#include "m1k_device.hpp"
#include "m1k_pipeline.hpp"
#include "m1k_game_object.hpp"
#include "m1k_camera.hpp"
#include "m1k_frame_info.hpp"

// std
#include <memory>

namespace m1k {

class SimpleRenderSystem {
   public:
    SimpleRenderSystem(M1kDevice &device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
    ~SimpleRenderSystem();

    // copy version delete
    SimpleRenderSystem(const SimpleRenderSystem&) = delete;
    SimpleRenderSystem &operator=(const SimpleRenderSystem&) = delete;

    void renderGameObjects(
        FrameInfo &frame_info,
        std::vector<M1kGameObject> &game_objects);

   private:
    void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
    void createPipeline(VkRenderPass render_pass);

    M1kDevice &m1k_device_;

    std::unique_ptr<M1kPipeline> m1k_pipeline_;
    VkPipelineLayout pipeline_layout_;
};

}
