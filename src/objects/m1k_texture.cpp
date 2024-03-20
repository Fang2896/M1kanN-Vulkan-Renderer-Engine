//
// Created by LIUFANG on 2023/12/20.
//

#include <stdexcept>
#include "m1k_texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//std
#include <cmath>


namespace m1k {

M1kTexture::M1kTexture(M1kDevice& device, const std::string& path)
    : m1k_device_(device), file_path(path)
{
    createTextureImage(path);
    createTextureImageView();
    createTextureSampler(VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_BORDER_COLOR_INT_OPAQUE_BLACK);
    createDescriptorImageInfo();
}

M1kTexture::~M1kTexture() {
    vkDestroyImage(m1k_device_.device(), m1k_texture_image_, nullptr);
    vkFreeMemory(m1k_device_.device(), m1k_texture_image_memory_, nullptr);

    vkDestroySampler(m1k_device_.device(), m1k_texture_sampler_, nullptr);
    vkDestroyImageView(m1k_device_.device(), m1k_texture_image_view_, nullptr);
}

VkDescriptorImageInfo& M1kTexture::getDescriptorImageInfo() {
    return m1k_image_info_;
}

const std::string& M1kTexture::getTextureFilePath() {
    return file_path;
}

void M1kTexture::createTextureImage(const std::string& path) {
    int tex_width, tex_height, tex_channels;
    stbi_uc* pixels = stbi_load(path.c_str(),
                                &tex_width,
                                &tex_height,
                                &tex_channels,
                                STBI_rgb_alpha);

    mip_levels_ = static_cast<uint32_t>(std::floor(std::log2(std::max(tex_width, tex_height)))) + 1;
    VkDeviceSize image_size = tex_width * tex_height * 4;

    if(!pixels) {
        throw std::runtime_error("M1k::ERR++++++++failed to load texture image!");
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

    createImage(tex_width, tex_height, mip_levels_,
                VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                m1k_texture_image_, m1k_texture_image_memory_);

    m1k_device_.transitionImageLayout(m1k_texture_image_, VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      mip_levels_);

    m1k_device_.copyBufferToImage(staging_buffer, m1k_texture_image_,
                                  static_cast<uint32_t>(tex_width),
                                  static_cast<uint32_t>(tex_height),
                                  1);
    // If we generate mipmap, we DO NOT need this!
//    m1k_device_.transitionImageLayout(m1k_texture_image_, VK_FORMAT_R8G8B8A8_UNORM,
//                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//                                      mip_levels_);

    vkDestroyBuffer(m1k_device_.device(), staging_buffer, nullptr);
    vkFreeMemory(m1k_device_.device(), staging_buffer_memory, nullptr);

    generateMipmaps(m1k_texture_image_, VK_FORMAT_R8G8B8A8_UNORM,
                    tex_width, tex_height, mip_levels_);
}

void M1kTexture::generateMipmaps(VkImage image, VkFormat image_format,
                     int32_t tex_width, int32_t tex_height, uint32_t mip_levels) {
    // Check if image format supports linear blitting
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(m1k_device_.getPhyDevice(),
                                        image_format, &formatProperties);

    if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        throw std::runtime_error("texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = m1k_device_.beginSingleTimeCommands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mip_width = tex_width;
    int32_t mip_height = tex_height;

    for (uint32_t i = 1; i < mip_levels; i++) {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,0, nullptr,
                             0, nullptr,
                             1, &barrier);

        VkImageBlit blit{};
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mip_width, mip_height, 1};
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mip_width > 1 ? mip_width / 2 : 1,
                              mip_height > 1 ? mip_height / 2 : 1, 1 };
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer,
                       image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &blit,
                       VK_FILTER_LINEAR);

        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,0, nullptr,
                             0, nullptr,
                             1, &barrier);

        if (mip_width > 1)
            mip_width /= 2;
        if (mip_height > 1)
            mip_height /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mip_levels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0,0, nullptr,
                         0, nullptr,
                         1, &barrier);

    m1k_device_.endSingleTimeCommands(commandBuffer);
}

void M1kTexture::createImage(uint32_t width, uint32_t height, uint32_t mip_levels,
                             VkFormat format,
                             VkImageTiling tiling, VkImageUsageFlags usage,
                             VkMemoryPropertyFlags properties, VkImage& image,
                             VkDeviceMemory& image_memory) {

    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = mip_levels;
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


void M1kTexture::createTextureImageView() {
    m1k_texture_image_view_ = m1k_device_.createImageView(m1k_texture_image_,
                                                          VK_FORMAT_R8G8B8A8_UNORM,
                                                          mip_levels_);
}

void M1kTexture::createTextureSampler(VkFilter filter_mode,
                                      VkSamplerAddressMode address_mode,
                                      VkBorderColor board_color) {

    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = filter_mode;
    sampler_info.minFilter = filter_mode;

    sampler_info.addressModeU = address_mode;
    sampler_info.addressModeV = address_mode;
    sampler_info.addressModeW = address_mode;

    sampler_info.anisotropyEnable = VK_TRUE;
    sampler_info.maxAnisotropy = 16;

    sampler_info.borderColor = board_color;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;  // shadow?

    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0;
    sampler_info.maxLod = static_cast<float>(mip_levels_);

    if (vkCreateSampler(m1k_device_.device(), &sampler_info, nullptr, &m1k_texture_sampler_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}


void M1kTexture::createDescriptorImageInfo() {
    m1k_image_info_.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    m1k_image_info_.imageView = m1k_texture_image_view_;
    m1k_image_info_.sampler = m1k_texture_sampler_;
}

}
