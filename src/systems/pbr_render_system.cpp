//
// Created by fangl on 2024/02/26.
//

#include "pbr_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <iostream>


namespace m1k {

PbrRenderSystem::PbrRenderSystem(M1kDevice &device, VkRenderPass render_pass,
                                 VkDescriptorSetLayout global_set_layout,
                                 VkDescriptorSetLayout pbr_set_layout)
    : m1k_device_(device)
{
    createPipelineLayout(global_set_layout, pbr_set_layout);
    createPipeline(render_pass);
}

PbrRenderSystem::~PbrRenderSystem() {
    vkDestroyPipelineLayout(m1k_device_.device(), pipeline_layout_, nullptr);
}

void PbrRenderSystem::createPipelineLayout(VkDescriptorSetLayout global_set_layout,
                                           VkDescriptorSetLayout pbr_set_layout) {
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout, pbr_set_layout};

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
    pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();

    if(vkCreatePipelineLayout(m1k_device_.device(),
                               &pipeline_layout_info,
                               nullptr,
                               &pipeline_layout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
    }
}

void PbrRenderSystem::createPipeline(VkRenderPass render_pass) {
    assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipeline_config{};
    M1kPipeline::defaultPipelineConfigInfo(pipeline_config);
    pipeline_config.render_pass = render_pass;
    pipeline_config.pipeline_layout = pipeline_layout_;
    m1k_pipeline_ = std::make_unique<M1kPipeline>(
        m1k_device_,
        pipeline_config,
        "./shaders/binaries/pbr_shader.vert.spv",
        "./shaders/binaries/pbr_shader.frag.spv");
}

void PbrRenderSystem::render(FrameInfo &frame_info) {
    m1k_pipeline_->bind(frame_info.command_buffer);

    vkCmdBindDescriptorSets(
        frame_info.command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout_,
        0, 1,
        &frame_info.global_descriptor_set,
        0, nullptr);

    for(auto& kv : frame_info.game_objects) {
        auto &obj = kv.second;

        // filter
        if(obj.getType() != GameObjectType::PbrObject) continue;

        obj.model->draw(frame_info.command_buffer, pipeline_layout_);
    }
}

}

