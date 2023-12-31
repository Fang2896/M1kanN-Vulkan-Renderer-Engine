//
// Created by fangl on 2023/12/2.
//

#pragma once

#include "m1k_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>


namespace m1k{

class M1kDescriptorSetLayout {
public:
    class Builder {
    public:
        Builder(M1kDevice &M1kDevice) : m1k_device_{M1kDevice} {}

        Builder &addBinding(
            uint32_t binding,
            VkDescriptorType descriptor_type,
            VkShaderStageFlags stage_flags,
            uint32_t count = 1);
        std::unique_ptr<M1kDescriptorSetLayout> build() const;

    private:
        M1kDevice &m1k_device_;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
    };

    M1kDescriptorSetLayout(
        M1kDevice &m1k_device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
    ~M1kDescriptorSetLayout();
    M1kDescriptorSetLayout(const M1kDescriptorSetLayout &) = delete;
    M1kDescriptorSetLayout &operator=(const M1kDescriptorSetLayout &) = delete;

    VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptor_set_layout_; }

private:
    M1kDevice &m1k_device_;
    VkDescriptorSetLayout descriptor_set_layout_;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

    friend class M1kDescriptorWriter;
};

class M1kDescriptorPool {
public:
    class Builder {
    public:
        Builder(M1kDevice &m1k_device) : m1k_device_{m1k_device} {}

        Builder &addPoolSize(VkDescriptorType descriptor_type, uint32_t count);
        Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
        Builder &setMaxSets(uint32_t count);
        std::unique_ptr<M1kDescriptorPool> build() const;

    private:
        M1kDevice &m1k_device_;
        std::vector<VkDescriptorPoolSize> pool_sizes_{};
        uint32_t max_sets_ = 1000;
        VkDescriptorPoolCreateFlags pool_flags_ = 0;
    };

    M1kDescriptorPool(
        M1kDevice &m1k_device,
        uint32_t max_sets,
        VkDescriptorPoolCreateFlags pool_flags,
        const std::vector<VkDescriptorPoolSize> &pool_sizes);
    ~M1kDescriptorPool();
    M1kDescriptorPool(const M1kDescriptorPool &) = delete;
    M1kDescriptorPool &operator=(const M1kDescriptorPool &) = delete;

    bool allocateDescriptor(
        const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet &descriptor) const;

    void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

    const VkDescriptorPool& getPool() const { return descriptor_pool_; }
    void resetPool();

private:
    M1kDevice &m1k_device_;
    VkDescriptorPool descriptor_pool_;

    friend class M1kDescriptorWriter;
};


class M1kDescriptorWriter {
public:
    M1kDescriptorWriter(M1kDescriptorSetLayout &set_layout, M1kDescriptorPool &pool);

    M1kDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *buffer_info);
    M1kDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *image_info);

    bool build(VkDescriptorSet &set);
    void overwrite(VkDescriptorSet &set);

private:
    M1kDescriptorSetLayout &set_layout_;
    M1kDescriptorPool &pool;
    std::vector<VkWriteDescriptorSet> writes;
};


}
