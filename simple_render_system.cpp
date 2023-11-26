//
// Created by fangl on 2023/11/26.
//

#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <iostream>


namespace m1k {

struct SimplePushConstantData {
    glm::mat4 transform{1.0f};
    alignas(16) glm::vec3 color;
};

SimpleRenderSystem::SimpleRenderSystem(M1kDevice &device, VkRenderPass render_pass)
    : m1k_device_(device) {
    createPipelineLayout();
    createPipeline(render_pass);
}

SimpleRenderSystem::~SimpleRenderSystem() {
    vkDestroyPipelineLayout(m1k_device_.device(), pipeline_layout_, nullptr);
}

void SimpleRenderSystem::createPipelineLayout() {

    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(SimplePushConstantData);

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pSetLayouts = nullptr;
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;

    if(vkCreatePipelineLayout(m1k_device_.device(),
                               &pipeline_layout_info,
                               nullptr,
                               &pipeline_layout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
    }
}

void SimpleRenderSystem::createPipeline(VkRenderPass render_pass) {
    assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipeline_config{};
    M1kPipeline::defaultPipelineConfigInfo(pipeline_config);
    pipeline_config.render_pass = render_pass;
    pipeline_config.pipeline_layout = pipeline_layout_;
    m1k_pipeline_ = std::make_unique<M1kPipeline>(
        m1k_device_,
        pipeline_config,
        "./shaders/binaries/simple_shader.vert.spv",
        "./shaders/binaries/simple_shader.frag.spv");
}

void SimpleRenderSystem::renderGameObjects(
    VkCommandBuffer command_buffer,
    std::vector<M1kGameObject> &game_objects,
    const M1kCamera &camera) {

    m1k_pipeline_->bind(command_buffer);

    for(auto& obj : game_objects) {
        obj.transform.rotation.y = glm::mod(obj.transform.rotation.y + 0.00018f, glm::two_pi<float>());
        obj.transform.rotation.x = glm::mod(obj.transform.rotation.x + 0.00009f, glm::two_pi<float>());

        SimplePushConstantData push{};
        push.color = obj.color;
        push.transform = camera.getProjection() * obj.transform.mat4();

        vkCmdPushConstants(command_buffer,
                           pipeline_layout_,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                           0,
                           sizeof(SimplePushConstantData),
                           &push);
        obj.model->bind(command_buffer);
        obj.model->draw(command_buffer);
    }
}

}

