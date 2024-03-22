//
// Created by LIUFANG on 2023/12/20.
//

#pragma once

#include "../core/m1k_device.hpp"

namespace m1k {

class M1kTexture {
   public:
    M1kTexture(M1kDevice& device, const std::string& path);
    ~M1kTexture();

    M1kTexture(const M1kTexture&) = delete;
    M1kTexture operator=(const M1kTexture&) = delete;

    VkDescriptorImageInfo& getDescriptorImageInfo();
    const std::string& getTextureFilePath();
    uint32_t getIndex() const { return index_; }

    static uint32_t next_texture_index;

private:
    void createTextureImage(const std::string& path);
    void createImage(uint32_t width, uint32_t height, uint32_t mip_levels,
                     VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image,
                     VkDeviceMemory& image_memory);
    void generateMipmaps(VkImage image, VkFormat image_format,
                         int32_t tex_width, int32_t tex_height, uint32_t mip_levels);

    void createTextureImageView();
    void createTextureSampler(VkFilter filter_mode,
                              VkSamplerAddressMode address_mode,
                              VkBorderColor board_color);
    void createDescriptorImageInfo();

    M1kDevice &m1k_device_;

    std::string file_path;

    uint32_t mip_levels_;
    VkImage m1k_texture_image_;
    VkDeviceMemory m1k_texture_image_memory_;

    VkImageView m1k_texture_image_view_;
    VkSampler m1k_texture_sampler_;

    VkDescriptorImageInfo m1k_image_info_{};

    const uint32_t index_;
};

}