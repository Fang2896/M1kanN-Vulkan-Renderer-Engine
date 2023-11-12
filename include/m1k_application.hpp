//
// Created by fangl on 2023/11/10.
//

#pragma once

#include "m1k_window.hpp"
#include "m1k_pipeline.hpp"
#include "m1k_device.hpp"
#include "m1k_swap_chain.hpp"

// std
#include <memory>

namespace m1k {

class M1kApplication {
   public:
    static constexpr int kWidth = 800;
    static constexpr int kHeight = 600;

    M1kApplication();
    ~M1kApplication();

    // copy version delete
    M1kApplication(const M1kApplication&) = delete;
    M1kApplication &operator=(const M1kApplication&) = delete;

    void run();

   private:
    void createPipelineLayout();
    void createPipeline();
    void createCommandBuffers();
    void drawFrame();

    M1kWindow m1k_window_{kWidth, kHeight, "Hello Vulkan"};
    M1kDevice m1k_device_{m1k_window_};
    M1kSwapChain m1k_swap_chain_{m1k_device_, m1k_window_.getExtent()};
    std::unique_ptr<M1kPipeline> m1k_pipeline_;
    VkPipelineLayout pipeline_layout_;
    std::vector<VkCommandBuffer> command_buffers;

};

}
