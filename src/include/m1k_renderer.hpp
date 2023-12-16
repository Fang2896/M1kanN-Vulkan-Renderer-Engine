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
    float getAspectRatio() const { return m1k_swap_chain_->extentAspectRatio(); }
    bool isFrameInProgress() const { return is_frame_started_; }

    VkCommandBuffer getCurrentCommandBuffer() const {
        assert(is_frame_started_ && "Cannot get command buffer_ when frame not in progress");
        return command_buffers_[current_frame_index_];
    }

    int getFrameIndex() const {
        assert(is_frame_started_ && "Cannot get frame index when frame not in progress");
        return current_frame_index_;
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

    uint32_t current_image_index_;
    int current_frame_index_{0};
    bool is_frame_started_{false};
};

}

