//
// Created by LIUFANG on 2023/12/20.
//

#pragma once

#include "core/m1k_device.hpp"

namespace m1k {

class M1kTexture {
   public:
    M1kTexture(M1kDevice& device, const std::string& path);
    ~M1kTexture();

    M1kTexture(const M1kTexture&) = delete;
    M1kTexture operator=(const M1kTexture&) = delete;

    VkDescriptorImageInfo& getDescriptorImageInfo();

private:
    void createTextureImage(const std::string& path);
    void createImage(uint32_t width, uint32_t height, uint32_t mip_level,
                     VkFormat format, VkImageTiling tiling,
                     VkImageUsageFlags usage, VkMemoryPropertyFlags properties,
                     VkImage& image, VkDeviceMemory& image_memory);

    void createTextureImageView();
    void createTextureSampler(VkFilter filter_mode,
                              VkSamplerAddressMode address_mode,
                              VkBorderColor board_color);
    void createDescriptorImageInfo();
    void generateMipmaps(VkImage image, VkFormat image_format, uint32_t tex_width, uint32_t tex_height, uint32_t mip_level);

    M1kDevice &m1k_device_;

    uint32_t mip_levels_{1};
    VkImage m1k_texture_image_;
    VkDeviceMemory m1k_texture_image_memory_;

    VkImageView m1k_texture_image_view_;
    VkSampler m1k_texture_sampler_;

    VkDescriptorImageInfo m1k_image_info_{};
};

}