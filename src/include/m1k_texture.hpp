//
// Created by LIUFANG on 2023/12/20.
//

#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "core/m1k_device.hpp"

namespace m1k {

class M1kTexture {
   public:
    M1kTexture(M1kDevice& device, const std::string& path);
    ~M1kTexture();

    M1kTexture(const M1kTexture&) = delete;
    M1kTexture operator=(const M1kTexture&) = delete;

   private:
    void createTextureImage(const std::string& path);
    void createImage(uint32_t width, uint32_t height, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image,
                     VkDeviceMemory& image_memory);

    M1kDevice &m1k_device_;
    VkImage m1k_texture_image_;
    VkDeviceMemory m1k_texture_image_memory_;
};

}