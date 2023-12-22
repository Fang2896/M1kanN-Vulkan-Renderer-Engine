//
// Created by fangl on 2023/11/10.
//

#pragma once


#include "ui/m1k_window.hpp"

// std lib headers
#include <string>
#include <vector>

namespace m1k {

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class M1kDevice {
   public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    M1kDevice(M1kWindow &window);
    ~M1kDevice();

    // Not copyable or movable
    M1kDevice(const M1kDevice &) = delete;
    M1kDevice operator=(const M1kDevice &) = delete;
    M1kDevice(M1kDevice &&) = delete;
    M1kDevice &operator=(M1kDevice &&) = delete;

    // only for imgui, to be optimized
    VkInstance getVkInstance() const { return instance_; }
    VkPhysicalDevice getPhyDevice() const { return physical_device_; };

    VkCommandPool getCommandPool() { return command_pool_; }
    VkDevice device() { return device_; }
    VkSurfaceKHR surface() { return surface_; }
    VkQueue graphicsQueue() { return graphics_queue_; }
    VkQueue presentQueue() { return present_queue_; }
    VkSampleCountFlagBits maxMSAASampleCount() { return msaa_samples_; }

    SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physical_device_); }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physical_device_); }
    VkFormat findSupportedFormat(
        const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    // Buffer Helper Functions
    void createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer &buffer,
        VkDeviceMemory &bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyBufferToImage(
        VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
    void createImageWithInfo(
        const VkImageCreateInfo &imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout old_layout, VkImageLayout new_layout,
                               uint32_t mip_levels=1);
    VkImageView createImageView(VkImage image, VkFormat format,
                                uint32_t mip_levels = 1,
                                VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT);

    VkPhysicalDeviceProperties properties;

   private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createCommandPool();

    // helper functions
    bool isDeviceSuitable(VkPhysicalDevice device);
    VkSampleCountFlagBits getMaxUsableSampleCount();
    std::vector<const char *> getRequiredExtensions();
    bool checkValidationLayerSupport();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
    void hasGlfwRequiredInstanceExtensions();
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    VkInstance instance_;
    VkDebugUtilsMessengerEXT debug_messenger_;
    VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
    M1kWindow &window_;
    VkCommandPool command_pool_;

    VkDevice device_;
    VkSurfaceKHR surface_;
    VkQueue graphics_queue_;
    VkQueue present_queue_;

    const std::vector<const char *> validation_layers_ = {"VK_LAYER_KHRONOS_validation"};

#ifdef __MACH__
    const std::vector<const char *> device_extensions_ = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_portability_subset"};
#elif
    const std::vector<const char *> device_extensions_ = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
#endif


    // tracer member variables
    VkSampleCountFlagBits msaa_samples_ = VK_SAMPLE_COUNT_1_BIT;
};

}