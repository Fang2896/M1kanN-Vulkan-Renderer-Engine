//
// Created by fangl on 2023/11/30.
//

#pragma once

#include "m1k_device.hpp"

namespace m1k {

class M1kBuffer {
public:
    M1kBuffer(
        M1kDevice& device,
        VkDeviceSize instance_size,
        uint32_t instance_count,
        VkBufferUsageFlags usage_flags,
        VkMemoryPropertyFlags memory_property_flags,
        VkDeviceSize min_offset_alignment = 1);
    ~M1kBuffer();

    M1kBuffer(const M1kBuffer&) = delete;
    M1kBuffer& operator=(const M1kBuffer&) = delete;

    VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    void unmap();

    void writeToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
    VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void writeToIndex(void* data, int index);
    VkResult flushIndex(int index);
    VkDescriptorBufferInfo descriptorInfoForIndex(int index);
    VkResult invalidateIndex(int index);

    VkBuffer getBuffer() const { return buffer_; }
    void* getMappedMemory() const { return mapped_; }
    uint32_t getInstanceCount() const { return instance_count_; }
    VkDeviceSize getInstanceSize() const { return instance_size_; }
    VkDeviceSize getAlignmentSize() const { return instance_size_; }
    VkBufferUsageFlags getUsageFlags() const { return usage_flags_; }
    VkMemoryPropertyFlags getMemoryPropertyFlags() const { return memory_property_flags_; }
    VkDeviceSize getBufferSize() const { return buffer_size_; }

private:
    static VkDeviceSize getAlignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment);

    M1kDevice& m1k_device_;
    void* mapped_ = nullptr;
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VkDeviceMemory memory_ = VK_NULL_HANDLE;

    VkDeviceSize buffer_size_;
    uint32_t instance_count_;
    VkDeviceSize instance_size_;
    VkDeviceSize alignment_size_;
    VkBufferUsageFlags usage_flags_;
    VkMemoryPropertyFlags memory_property_flags_;
};

}  // namespace m1k
