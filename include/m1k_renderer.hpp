//
// Created by fangl on 2023/11/25.
//

#pragma once


#include "m1k_window.hpp"
#include "m1k_device.hpp"
#include "m1k_swap_chain.hpp"

// std
#include <memory>
#include <cassert>
#include <vector>

namespace m1k {

class M1kRenderer {
   public:
    M1kRenderer(M1kWindow &window,M1kDevice &device);
    ~M1kRenderer();

    // copy version delete
    M1kRenderer(const M1kRenderer&) = delete;
    M1kRenderer &operator=(const M1kRenderer&) = delete;

    VkRenderPass getSwapChainRenderPass() const { return m1k_swap_chain_->getRenderPass(); }
    bool isFrameInProgress() const { return is_frame_started; }

    VkCommandBuffer getCurrentCommandBuffer() const {
        assert(is_frame_started && "Cannot get command buffer when frame not in progress");
        return command_buffers_[current_frame_index];
    }

    int getFrameIndex() const {
        assert(is_frame_started && "Cannot get frame index when frame not in progress");
        return current_frame_index;
    }

    VkCommandBuffer beginFrame();
    void endFrame();
    void beginSwapChainRenderPass(VkCommandBuffer command_buffer);
    void endSwapChainRenderPass(VkCommandBuffer command_buffer);

   private:
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    M1kWindow& m1k_window_;
    M1kDevice& m1k_device_;
    std::unique_ptr<M1kSwapChain> m1k_swap_chain_;
    std::vector<VkCommandBuffer> command_buffers_;

    uint32_t current_image_index;
    int current_frame_index{0};
    bool is_frame_started{false};
};

}

