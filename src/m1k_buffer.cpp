//
// Created by fangl on 2023/11/30.
//

/*
 * Encapsulates a vulkan buffer_
 *
 * Initially based off VulkanBuffer by Sascha Willems -
 * https://github.com/SaschaWillems/Vulkan/blob/master/base/VulkanBuffer.h
 */

#include "m1k_buffer.hpp"

// std
#include <cassert>
#include <cstring>

namespace m1k {

/**
 * Returns the minimum instance_ size required to be compatible with devices min_offset_alignment
 *
 * @param instance_size The size of an instance_
 * @param min_offset_alignment The minimum required alignment, in bytes, for the offset member (eg
 * minUniformBufferOffsetAlignment)
 *
 * @return VkResult of the buffer_ mapping call
 */
VkDeviceSize M1kBuffer::getAlignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment) {
    if (min_offset_alignment > 0) {
        return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
    }
    return instance_size;
}

M1kBuffer::M1kBuffer(
    M1kDevice& device,
    VkDeviceSize instance_size,
    uint32_t instance_count,
    VkBufferUsageFlags usage_flags,
    VkMemoryPropertyFlags memory_property_flags,
    VkDeviceSize min_offset_alignment)
    : m1k_device_{device},
      instance_size_{instance_size},
      instance_count_{instance_count},
      usage_flags_{usage_flags},
      memory_property_flags_{memory_property_flags} {
    alignment_size_ = getAlignment(instance_size, min_offset_alignment);
    buffer_size_ = alignment_size_ * instance_count;
    device.createBuffer(buffer_size_, usage_flags, memory_property_flags, buffer_, memory_);
}

M1kBuffer::~M1kBuffer() {
    unmap();
    vkDestroyBuffer(m1k_device_.device(), buffer_, nullptr);
    vkFreeMemory(m1k_device_.device(), memory_, nullptr);
}

/**
 * Map a memory_ range of this buffer_. If successful, mapped_ points to the specified buffer_ range.
 *
 * @param size (Optional) Size of the memory_ range to map. Pass VK_WHOLE_SIZE to map the complete
 * buffer_ range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the buffer_ mapping call
 */
VkResult M1kBuffer::map(VkDeviceSize size, VkDeviceSize offset) {
    assert(buffer_ && memory_ && "Called map on buffer_ before create");
    return vkMapMemory(m1k_device_.device(), memory_, offset, size, 0, &mapped_);
}

/**
 * Unmap a mapped_ memory_ range
 *
 * @note Does not return a result as vkUnmapMemory can't fail
 */
void M1kBuffer::unmap() {
    if (mapped_) {
        vkUnmapMemory(m1k_device_.device(), memory_);
        mapped_ = nullptr;
    }
}

/**
 * Copies the specified data to the mapped_ buffer_. Default value writes whole buffer_ range
 *
 * @param data Pointer to the data to copy
 * @param size (Optional) Size of the data to copy. Pass VK_WHOLE_SIZE to flush the complete buffer_
 * range.
 * @param offset (Optional) Byte offset from beginning of mapped_ region
 *
 */
void M1kBuffer::writeToBuffer(void *data, VkDeviceSize size, VkDeviceSize offset) {
    assert(mapped_ && "Cannot copy to unmapped buffer_");

    if (size == VK_WHOLE_SIZE) {
        memcpy(mapped_, data, buffer_size_);
    } else {
        char *memOffset = (char *)mapped_;
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

/**
 * Flush a memory_ range of the buffer_ to make it visible to the device_
 *
 * @note Only required for non-coherent memory_
 *
 * @param size (Optional) Size of the memory_ range to flush. Pass VK_WHOLE_SIZE to flush the
 * complete buffer_ range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
VkResult M1kBuffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory_;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkFlushMappedMemoryRanges(m1k_device_.device(), 1, &mappedRange);
}

/**
 * Invalidate a memory_ range of the buffer_ to make it visible to the host
 *
 * @note Only required for non-coherent memory_
 *
 * @param size (Optional) Size of the memory_ range to invalidate. Pass VK_WHOLE_SIZE to invalidate
 * the complete buffer_ range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
VkResult M1kBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory_;
    mappedRange.offset = offset;
    mappedRange.size = size;
    return vkInvalidateMappedMemoryRanges(m1k_device_.device(), 1, &mappedRange);
}

/**
 * Create a buffer_ info descriptor
 *
 * @param size (Optional) Size of the memory_ range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkDescriptorBufferInfo of specified offset and range
 */
VkDescriptorBufferInfo M1kBuffer::descriptorInfo(VkDeviceSize size, VkDeviceSize offset) {
    return VkDescriptorBufferInfo{
        buffer_,
        offset,
        size,
    };
}

/**
 * Copies "instance_size_" bytes of data to the mapped_ buffer_ at an offset of index * alignment_size_
 *
 * @param data Pointer to the data to copy
 * @param index Used in offset calculation
 *
 */
void M1kBuffer::writeToIndex(void *data, int index) {
    writeToBuffer(data, instance_size_, index * alignment_size_);
}

/**
 *  Flush the memory_ range at index * alignment_size_ of the buffer_ to make it visible to the device_
 *
 * @param index Used in offset calculation
 *
 */
VkResult M1kBuffer::flushIndex(int index) { return flush(alignment_size_, index * alignment_size_); }

/**
 * Create a buffer_ info descriptor
 *
 * @param index Specifies the region given by index * alignment_size_
 *
 * @return VkDescriptorBufferInfo for instance_ at index
 */
VkDescriptorBufferInfo M1kBuffer::descriptorInfoForIndex(int index) {
    return descriptorInfo(alignment_size_, index * alignment_size_);
}

/**
 * Invalidate a memory_ range of the buffer_ to make it visible to the host
 *
 * @note Only required for non-coherent memory_
 *
 * @param index Specifies the region to invalidate: index * alignment_size_
 *
 * @return VkResult of the invalidate call
 */
VkResult M1kBuffer::invalidateIndex(int index) {
    return invalidate(alignment_size_, index * alignment_size_);
}

}  // namespace lve
