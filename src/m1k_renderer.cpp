//
// Created by fangl on 2023/11/25.
//

#include "m1k_renderer.hpp"

// std
#include <stdexcept>
#include <array>


namespace m1k {

M1kRenderer::M1kRenderer(M1kWindow &window, M1kDevice &device)
    : m1k_window_(window), m1k_device_(device) {

    recreateSwapChain();
    createCommandBuffers();
}

M1kRenderer::~M1kRenderer() {
    freeCommandBuffers();
}

void M1kRenderer::recreateSwapChain() {
    auto extent = m1k_window_.getExtent();
    while(extent.width == 0 || extent.height == 0) {
        extent = m1k_window_.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m1k_device_.device());

    if(m1k_swap_chain_ == nullptr) {
        m1k_swap_chain_ = std::make_unique<M1kSwapChain>(m1k_device_, extent);
    } else {
        std::shared_ptr<M1kSwapChain> old_swap_chain = std::move(m1k_swap_chain_);
        m1k_swap_chain_ = std::make_unique<M1kSwapChain>(m1k_device_, extent, old_swap_chain);

        if(!old_swap_chain->compareSwapFormat(*m1k_swap_chain_.get())) {
            throw std::runtime_error("Swap chain image(or depth) format has changed!");
        }
    }
    // ...
}

void M1kRenderer::createCommandBuffers() {
    command_buffers_.resize(M1kSwapChain::MAX_FRAMES_IN_FLIGHT);

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

void M1kRenderer::freeCommandBuffers() {
    vkFreeCommandBuffers(
        m1k_device_.device(),
        m1k_device_.getCommandPool(),
        static_cast<uint32_t>(command_buffers_.size()),
        command_buffers_.data());

    command_buffers_.clear();
}

VkCommandBuffer M1kRenderer::beginFrame() {
    assert(!is_frame_started && "Cannot call beginFrame while already in progress");

    auto result = m1k_swap_chain_->acquireNextImage(&current_image_index);

    if(result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
    }

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    is_frame_started = true;

    auto command_buffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if(vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer");
    }

    return command_buffer;
}

void M1kRenderer::endFrame() {
    assert(is_frame_started && "Cannot call endFrame while frame is not in progress");
    auto command_buffer = getCurrentCommandBuffer();

    if(vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer");
    }

    auto result = m1k_swap_chain_->submitCommandBuffers(&command_buffer, &current_image_index);

    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m1k_window_.wasWindowResized()) {
        m1k_window_.resetWindowResizedFlag();
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image");
    }

    is_frame_started = false;
    current_frame_index = (current_frame_index + 1) % M1kSwapChain::MAX_FRAMES_IN_FLIGHT;
}

void M1kRenderer::beginSwapChainRenderPass(VkCommandBuffer command_buffer) {
    assert(is_frame_started && "Cannot call beginSwapChainRenderPass if frame is not in progress");
    assert(
        command_buffer == getCurrentCommandBuffer() &&
        "Cannot begin render pass on command buffer from a different frame");

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = m1k_swap_chain_->getRenderPass();
    render_pass_info.framebuffer = m1k_swap_chain_->getFrameBuffer(current_image_index);

    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = m1k_swap_chain_->getSwapChainExtent();  // not windows extent

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = {0.01f, 0.01f, 0.01f, 1.0f};   // attachment
    clear_values[1].depthStencil = {1.0f, 0};
    render_pass_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m1k_swap_chain_->getSwapChainExtent().width);
    viewport.height = static_cast<float>(m1k_swap_chain_->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, m1k_swap_chain_->getSwapChainExtent()};
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

}

void M1kRenderer::endSwapChainRenderPass(VkCommandBuffer command_buffer) {
    assert(is_frame_started && "Cannot call endSwapChainRenderPass if frame is not in progress");
    assert(
        command_buffer == getCurrentCommandBuffer() &&
        "Cannot end render pass on command buffer from a different frame");

    vkCmdEndRenderPass(command_buffer);
}

}


