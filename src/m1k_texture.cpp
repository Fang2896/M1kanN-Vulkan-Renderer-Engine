//
// Created by LIUFANG on 2023/12/20.
//

#include "m1k_texture.hpp"

namespace m1k {

M1kTexture::M1kTexture(M1kDevice& device, const std::string& path)
    : m1k_device_(device)
{
    createTextureImage(path);
}

void M1kTexture::createTextureImage(const std::string& path) {
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load(path.c_str(),
                                &tex_width,
                                &tex_height,
                                &tex_channels,
                                STBI_rgb_alpha);

    VkDeviceSize image_size = tex_width * tex_height * 4;

    if(!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    m1k_device_.createBuffer(image_size,
                             VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             staging_buffer, staging_buffer_memory);
    void* data;
    vkMapMemory(m1k_device_.device(), staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(image_size));
    vkUnmapMemory(m1k_device_.device(), staging_buffer_memory);

    stbi_image_free(pixels);

    createImage(tex_width, tex_height,
                VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m1k_texture_image_, m1k_texture_image_memory_);

    m1k_device_.transitionImageLayout(m1k_texture_image_, VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_LAYOUT_PREINITIALIZED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    m1k_device_.copyBufferToImage(staging_buffer, m1k_texture_image_,
                                  static_cast<uint32_t>(tex_width),
                                  static_cast<uint32_t>(tex_height),
                                  1);
    m1k_device_.transitionImageLayout(m1k_texture_image_, VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void M1kTexture::createImage(uint32_t width, uint32_t height, VkFormat format,
                             VkImageTiling tiling, VkImageUsageFlags usage,
                             VkMemoryPropertyFlags properties, VkImage& image,
                             VkDeviceMemory& image_memory) {

    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags = 0; // Optional
    m1k_device_.createImageWithInfo(image_info, properties,
                                    image, image_memory);
}

}
