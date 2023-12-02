//
// Created by fangl on 2023/12/2.
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

class PointLightSystem {
public:
    PointLightSystem(M1kDevice &device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout);
    ~PointLightSystem();

    // copy version delete
    PointLightSystem(const PointLightSystem&) = delete;
    PointLightSystem &operator=(const PointLightSystem&) = delete;

    void update(FrameInfo &frame_info, GlobalUbo &ubo);
    void render(FrameInfo &frame_info);

private:
    void createPipelineLayout(VkDescriptorSetLayout global_set_layout);
    void createPipeline(VkRenderPass render_pass);

    M1kDevice &m1k_device_;

    std::unique_ptr<M1kPipeline> m1k_pipeline_;
    VkPipelineLayout pipeline_layout_;
};

}
