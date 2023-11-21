//
// Created by fangl on 2023/11/10.
//

#include "m1k_application.hpp"

// std
#include <stdexcept>
#include <array>


namespace m1k {

M1kApplication::M1kApplication() {
    loadModels();
    createPipelineLayout();
    createPipeline();
    createCommandBuffers();
}

M1kApplication::~M1kApplication() {
    vkDestroyPipelineLayout(m1k_device_.device(), pipeline_layout_, nullptr);
}

void M1kApplication::run() {
    while(!m1k_window_.shouldClose()) {
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(m1k_device_.device());
}

void M1kApplication::loadModels() {
    std::vector<M1kModel::Vertex> vertices {
        {{0.0f, -0.5f}},
        {{0.5f, 0.5f}},
        {{-0.5f, 0.5f}}
    };

    m1K_model_ = std::make_unique<M1kModel>(m1k_device_, vertices);
}

void M1kApplication::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pSetLayouts = nullptr;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = nullptr;

    if(vkCreatePipelineLayout(m1k_device_.device(),
                               &pipeline_layout_info,
                               nullptr,
                               &pipeline_layout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
    }
}

void M1kApplication::createPipeline() {
    PipelineConfigInfo pipeline_config{};
    M1kPipeline::defaultPipelineConfigInfo(
        pipeline_config,
        m1k_swap_chain_.width(),
        m1k_swap_chain_.height());
    pipeline_config.render_pass = m1k_swap_chain_.getRenderPass();
    pipeline_config.pipeline_layout = pipeline_layout_;
    m1k_pipeline_ = std::make_unique<M1kPipeline>(
        m1k_device_,
        pipeline_config,
        "./shaders/binaries/simple_shader.vert.spv",
        "./shaders/binaries/simple_shader.frag.spv");
}

void M1kApplication::createCommandBuffers() {
    command_buffers_.resize(m1k_swap_chain_.imageCount());

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = m1k_device_.getCommandPool();
    alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

    if(vkAllocateCommandBuffers(m1k_device_.device(), &alloc_info,
                                 command_buffers_.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers");
    }

    for(int i = 0; i < command_buffers_.size(); i++) {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if(vkBeginCommandBuffer(command_buffers_[i], &begin_info) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer");
        }

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = m1k_swap_chain_.getRenderPass();
        render_pass_info.framebuffer = m1k_swap_chain_.getFrameBuffer(i);

        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = m1k_swap_chain_.getSwapChainExtent();  // not windows extent

        std::array<VkClearValue, 2> clear_values{};
        clear_values[0].color = {0.1f, 0.1f, 0.1f, 1.0f};   // attachment
        clear_values[1].depthStencil = {1.0f, 0};
        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
        render_pass_info.pClearValues = clear_values.data();

        vkCmdBeginRenderPass(command_buffers_[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        m1k_pipeline_->bind(command_buffers_[i]);
        m1K_model_->bind(command_buffers_[i]);
        m1K_model_->draw(command_buffers_[i]);

        vkCmdEndRenderPass(command_buffers_[i]);
        if(vkEndCommandBuffer(command_buffers_[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer");
        }
    }
}

void M1kApplication::drawFrame() {
    uint32_t image_index;
    auto result = m1k_swap_chain_.acquireNextImage(&image_index);

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    result = m1k_swap_chain_.submitCommandBuffers(&command_buffers_[image_index], &image_index);

    if(result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image");
    }
}

}
