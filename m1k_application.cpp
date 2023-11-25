//
// Created by fangl on 2023/11/10.
//

#include "m1k_application.hpp"

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
    glm::mat2 transform{1.0f};
    glm::vec2 offset;
    alignas(16) glm::vec3 color;
};

M1kApplication::M1kApplication() {
    loadGameObjects();
    createPipelineLayout();
    recreateSwapChain();
    createCommandBuffers();
}

M1kApplication::~M1kApplication() {
    vkDestroyPipelineLayout(m1k_device_.device(), pipeline_layout_, nullptr);
}

void M1kApplication::run() {
    std::cout << "max push constant size: " << m1k_device_.properties.limits.maxPushConstantsSize << "\n";

    while(!m1k_window_.shouldClose()) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(m1k_device_.device());
}

void M1kApplication::loadGameObjects() {
    std::vector<M1kModel::Vertex> vertices {
        {{0.0f, -0.5f}, {1,0,0}},
        {{0.5f, 0.5f}, {0,1,0}},
        {{-0.5f, 0.5f}, {0,0,1}}
    };

    auto m1K_model = std::make_shared<M1kModel>(m1k_device_, vertices);

    auto triangle = M1kGameObject::createGameObject();
    triangle.model = m1K_model;
    triangle.color = {.1f, .8f, .1f};
    triangle.transform_2d_component.translation.x = .2f;
    triangle.transform_2d_component.scale = {2.0f, 0.5f};
    triangle.transform_2d_component.rotation = 0.25f * glm::two_pi<float>();

    game_objects_.push_back(std::move(triangle));
}

void M1kApplication::createPipelineLayout() {

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

void M1kApplication::createPipeline() {
    assert(m1k_swap_chain_ != nullptr && "Cannot create pipeline before swap chain");
    assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipeline_config{};
    M1kPipeline::defaultPipelineConfigInfo(pipeline_config);
    pipeline_config.render_pass = m1k_swap_chain_->getRenderPass();
    pipeline_config.pipeline_layout = pipeline_layout_;
    m1k_pipeline_ = std::make_unique<M1kPipeline>(
        m1k_device_,
        pipeline_config,
        "./shaders/binaries/simple_shader.vert.spv",
        "./shaders/binaries/simple_shader.frag.spv");
}

void M1kApplication::recreateSwapChain() {
    auto extent = m1k_window_.getExtent();
    while(extent.width == 0 || extent.height == 0) {
        extent = m1k_window_.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m1k_device_.device());

//    // if omit this line, resize will crash with "fail create swap chain"
//    m1k_swap_chain_.reset(nullptr);

    if(m1k_swap_chain_ == nullptr) {
        m1k_swap_chain_ = std::make_unique<M1kSwapChain>(m1k_device_, extent);
    } else {
        m1k_swap_chain_ = std::make_unique<M1kSwapChain>(m1k_device_, extent, std::move(m1k_swap_chain_));
        if(m1k_swap_chain_->imageCount() != command_buffers_.size()) {
            freeCommandBuffers();
            createCommandBuffers();
        }
    }

    createPipeline();
}

void M1kApplication::createCommandBuffers() {
    command_buffers_.resize(m1k_swap_chain_->imageCount());

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = m1k_device_.getCommandPool();
    alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

    if(vkAllocateCommandBuffers(m1k_device_.device(), &alloc_info,
                                 command_buffers_.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers");
    }
}

void M1kApplication::freeCommandBuffers() {
    vkFreeCommandBuffers(
        m1k_device_.device(),
        m1k_device_.getCommandPool(),
        static_cast<uint32_t>(command_buffers_.size()),
        command_buffers_.data());

    command_buffers_.clear();
}

void M1kApplication::recordCommandBuffer(int image_index) {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if(vkBeginCommandBuffer(command_buffers_[image_index], &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer");
    }

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = m1k_swap_chain_->getRenderPass();
    render_pass_info.framebuffer = m1k_swap_chain_->getFrameBuffer(image_index);

    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = m1k_swap_chain_->getSwapChainExtent();  // not windows extent

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {0.01f, 0.01f, 0.01f, 1.0f};   // attachment
    clear_values[1].depthStencil = {1.0f, 0};
    render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(command_buffers_[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m1k_swap_chain_->getSwapChainExtent().width);
    viewport.height = static_cast<float>(m1k_swap_chain_->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, m1k_swap_chain_->getSwapChainExtent()};
    vkCmdSetViewport(command_buffers_[image_index], 0, 1, &viewport);
    vkCmdSetScissor(command_buffers_[image_index], 0, 1, &scissor);

    renderGameObjects(command_buffers_[image_index]);

    vkCmdEndRenderPass(command_buffers_[image_index]);
    if(vkEndCommandBuffer(command_buffers_[image_index]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer");
    }
}

void M1kApplication::renderGameObjects(VkCommandBuffer command_buffer) {
    m1k_pipeline_->bind(command_buffer);

    for(auto& obj : game_objects_) {
        obj.transform_2d_component.rotation = glm::mod(obj.transform_2d_component.rotation + 0.01f, glm::two_pi<float>());

        SimplePushConstantData push{};
        push.offset = obj.transform_2d_component.translation;
        push.color = obj.color;
        push.transform = obj.transform_2d_component.mat2();

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

void M1kApplication::drawFrame() {
    uint32_t image_index;
    auto result = m1k_swap_chain_->acquireNextImage(&image_index);

    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    }

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    recordCommandBuffer(image_index);
    result = m1k_swap_chain_->submitCommandBuffers(&command_buffers_[image_index], &image_index);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m1k_window_.wasWindowResized()) {
        m1k_window_.resetWindowResizedFlag();
        recreateSwapChain();
        return;
    }

    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image");
    }
}

}
