//
// Created by fangl on 2023/11/12.
//

#pragma once

#include "m1k_device.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace m1k {

class M1kSwapChain {
   public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

     M1kSwapChain( M1kDevice &deviceRef, VkExtent2D windowExtent);
     M1kSwapChain( M1kDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<M1kSwapChain> previous);
     ~M1kSwapChain();

    M1kSwapChain(const  M1kSwapChain &) = delete;
    M1kSwapChain operator=(const  M1kSwapChain &) = delete;

    VkFramebuffer getFrameBuffer(int index) { return swap_chain_framebuffers_[index]; }
    VkRenderPass getRenderPass() { return render_pass_; }
    VkImageView getImageView(int index) { return swap_chain_image_views_[index]; }
    size_t imageCount() { return swap_chain_images_.size(); }
    VkFormat getSwapChainImageFormat() { return swap_chain_image_format_; }
    VkExtent2D getSwapChainExtent() { return swap_chain_extent_; }
    uint32_t width() { return swap_chain_extent_.width; }
    uint32_t height() { return swap_chain_extent_.height; }

    float extentAspectRatio() {
        return static_cast<float>(swap_chain_extent_.width) / static_cast<float>(swap_chain_extent_.height);
    }
    VkFormat findDepthFormat();

    VkResult acquireNextImage(uint32_t *imageIndex);
    VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

    bool compareSwapFormat(const M1kSwapChain &swap_chain) const {
        return swap_chain.swap_chain_depth_format_ == swap_chain_depth_format_ &&
                swap_chain.swap_chain_image_format_ == swap_chain_image_format_;
    }

   private:
    void init();
    void createSwapChain();
    void createImageViews();
    void createDepthResources();
    void createRenderPass();
    void createFramebuffers();
    void createSyncObjects();

    // Helper functions
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkFormat swap_chain_image_format_;
    VkFormat swap_chain_depth_format_;
    VkExtent2D swap_chain_extent_;

    std::vector<VkFramebuffer> swap_chain_framebuffers_;
    VkRenderPass render_pass_;

    VkImage depth_images_;
    VkDeviceMemory depth_image_memorys_;
    VkImageView depth_image_views_;

    std::vector<VkImage> swap_chain_images_;
    std::vector<VkImageView> swap_chain_image_views_;

    M1kDevice &device_;
    VkExtent2D window_extent_;

    VkSwapchainKHR swap_chain_;
    std::shared_ptr<M1kSwapChain> old_swap_chain_;

    std::vector<VkSemaphore> image_available_semaphores_;
    std::vector<VkSemaphore> render_finished_semaphores_;
    std::vector<VkFence> in_flight_fences_;
    std::vector<VkFence> images_in_flight_;
    size_t current_frame_ = 0;
};

}  // namespace m1k
