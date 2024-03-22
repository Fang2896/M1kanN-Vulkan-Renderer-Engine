//
// Created by fangl on 2023/12/2.
//

#include "m1k_descriptor.hpp"

// std
#include <cassert>
#include <stdexcept>
#include <map>


namespace m1k {

// *************** Descriptor Set Layout Builder *********************

M1kDescriptorSetLayout::Builder &M1kDescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptor_type,
    VkShaderStageFlags stage_flags,
    uint32_t count)
{
    assert(bindings_.count(binding) == 0 && "Binding already in use");
    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = binding;
    layout_binding.descriptorCount = count;

    layout_binding.descriptorType = descriptor_type;
    layout_binding.pImmutableSamplers = nullptr;
    layout_binding.stageFlags = stage_flags;
    bindings_[binding] = layout_binding;
    return *this;
}

std::unique_ptr<M1kDescriptorSetLayout> M1kDescriptorSetLayout::Builder::build() const {
    return std::make_unique<M1kDescriptorSetLayout>(m1k_device_, bindings_);
}

std::unique_ptr<M1kDescriptorSetLayout> M1kDescriptorSetLayout::Builder::build_for_bindless() const {
    return std::make_unique<M1kDescriptorSetLayout>(m1k_device_,
                                                    bindings_,
                                                    VK_TRUE);
}

// *************** Descriptor Set Layout *********************

M1kDescriptorSetLayout::M1kDescriptorSetLayout(
    M1kDevice &m1k_device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : m1k_device_{m1k_device}, bindings{bindings}
{
    std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};
    std::map<uint32_t , VkDescriptorSetLayoutBinding> temp_bindings{};
    for (auto kv : bindings) {
        temp_bindings[kv.first] = kv.second;
    }
    for (auto kv : temp_bindings) {
        set_layout_bindings.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
    descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
    descriptor_set_layout_info.pBindings = set_layout_bindings.data();

    if (vkCreateDescriptorSetLayout(
        m1k_device.device(),
        &descriptor_set_layout_info,
        nullptr,
        &descriptor_set_layout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

M1kDescriptorSetLayout::M1kDescriptorSetLayout(
    M1kDevice &m1k_device,
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings,
    bool isBindless)
    : m1k_device_{m1k_device}, bindings{bindings}
{
    if(!isBindless)
        throw std::runtime_error("M1k::ERR--------Use Bindless Wrongly!");

    std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings{};
    std::map<uint32_t , VkDescriptorSetLayoutBinding> temp_bindings{};
    for (auto kv : bindings) {
        temp_bindings[kv.first] = kv.second;
    }
    for (auto kv : temp_bindings) {
        set_layout_bindings.push_back(kv.second);
    }

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_info{};
    descriptor_set_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_info.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());
    descriptor_set_layout_info.pBindings = set_layout_bindings.data();
    descriptor_set_layout_info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT;

    VkDescriptorBindingFlags bindless_flags =
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT |
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT;
    \
    uint32_t binding_size = static_cast<uint32_t>(set_layout_bindings.size());
    std::vector<VkDescriptorBindingFlags> binding_flags {binding_size};
    for(auto& binding_flag : binding_flags) {
        binding_flag = bindless_flags;
    }

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
        nullptr };
    extended_info.bindingCount = binding_size;
    extended_info.pBindingFlags = binding_flags.data();
    descriptor_set_layout_info.pNext = &extended_info;

    if (vkCreateDescriptorSetLayout(
            m1k_device.device(),
            &descriptor_set_layout_info,
            nullptr,
            &descriptor_set_layout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

M1kDescriptorSetLayout::~M1kDescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(m1k_device_.device(), descriptor_set_layout_, nullptr);
}


// *************** Descriptor Pool Builder *********************

M1kDescriptorPool::Builder &M1kDescriptorPool::Builder::addPoolSize(
    VkDescriptorType descriptor_type, uint32_t count) {

    pool_sizes_.push_back({descriptor_type, count});
    return *this;
}

M1kDescriptorPool::Builder &M1kDescriptorPool::Builder::setPoolFlags(
    VkDescriptorPoolCreateFlags flags) {
    pool_flags_ = flags;
    return *this;
}
M1kDescriptorPool::Builder &M1kDescriptorPool::Builder::setMaxSets(uint32_t count) {
    max_sets_ = count;
    return *this;
}

std::unique_ptr<M1kDescriptorPool> M1kDescriptorPool::Builder::build() const {
    return std::make_unique<M1kDescriptorPool>(m1k_device_,
                                               max_sets_,
                                               pool_flags_,
                                               pool_sizes_);
}


// *************** Descriptor Pool *********************

M1kDescriptorPool::M1kDescriptorPool(
    M1kDevice &m1k_device,
    uint32_t max_sets,
    VkDescriptorPoolCreateFlags pool_flags,
    const std::vector<VkDescriptorPoolSize> &pool_sizes)
        : m1k_device_{m1k_device} {

    VkDescriptorPoolCreateInfo descriptor_pool_info{};
    descriptor_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    descriptor_pool_info.pPoolSizes = pool_sizes.data();
    descriptor_pool_info.maxSets = max_sets;
    descriptor_pool_info.flags = pool_flags;

    if (vkCreateDescriptorPool(m1k_device.device(), &descriptor_pool_info, nullptr, &descriptor_pool_) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool_!");
    }
}

M1kDescriptorPool::~M1kDescriptorPool() {
    vkDestroyDescriptorPool(m1k_device_.device(), descriptor_pool_, nullptr);
}

bool M1kDescriptorPool::allocateDescriptor(
    const VkDescriptorSetLayout descriptor_set_layout, VkDescriptorSet &descriptor) const {
    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = descriptor_pool_;
    alloc_info.pSetLayouts = &descriptor_set_layout;
    alloc_info.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
    // a new pool_ whenever an old pool_ fills up. But this is beyond our current scope
    if (vkAllocateDescriptorSets(m1k_device_.device(), &alloc_info, &descriptor) != VK_SUCCESS) {
        return false;
    }
    return true;
}

void M1kDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
    vkFreeDescriptorSets(
        m1k_device_.device(),
        descriptor_pool_,
        static_cast<uint32_t>(descriptors.size()),
        descriptors.data());
}

void M1kDescriptorPool::resetPool() {
    vkResetDescriptorPool(m1k_device_.device(), descriptor_pool_, 0);
}


// *************** Descriptor Writer *********************

M1kDescriptorWriter::M1kDescriptorWriter(M1kDescriptorSetLayout &set_layout, M1kDescriptorPool &pool)
    : set_layout_{set_layout}, pool_{pool} {}

M1kDescriptorWriter &M1kDescriptorWriter::writeBuffer(
    uint32_t binding, VkDescriptorBufferInfo *buffer_info) {
    assert(set_layout_.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &binding_description = set_layout_.bindings[binding];

    assert(
        binding_description.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = binding_description.descriptorType;
    write.dstBinding = binding;
    write.pBufferInfo = buffer_info;
    write.descriptorCount = 1;

    writes_.push_back(write);
    return *this;
}

M1kDescriptorWriter &M1kDescriptorWriter::writeImage(
    uint32_t binding, VkDescriptorImageInfo *image_info) {
    assert(set_layout_.bindings.count(binding) == 1 && "Layout does not contain specified binding");

    auto &binding_description = set_layout_.bindings[binding];

    assert(
        binding_description.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.descriptorType = binding_description.descriptorType;
    write.dstBinding = binding;
    write.pImageInfo = image_info;
    write.descriptorCount = 1;

    writes_.push_back(write);
    return *this;
}

bool M1kDescriptorWriter::build(VkDescriptorSet &set) {
    bool success =
        pool_.allocateDescriptor(set_layout_.getDescriptorSetLayout(), set);
    if (!success) {
        return false;
    }
    overwrite(set);
    return true;
}

void M1kDescriptorWriter::overwrite(VkDescriptorSet &set) {
    for (auto &write : writes_) {
        write.dstSet = set;
    }
    vkUpdateDescriptorSets(pool_.m1k_device_.device(), writes_.size(),
                           writes_.data(), 0, nullptr);
}

void M1kDescriptorPool::getBindlessDescriptorSet(
    M1kDevice &device,
    VkDescriptorPool bindless_descriptor_pool,
    VkDescriptorSetLayout bindless_descriptor_set_layout,
    VkDescriptorSet &set,
    uint32_t maxBindlessResource)
{
    VkDescriptorSetAllocateInfo alloc_info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO
    };
    alloc_info.descriptorPool = bindless_descriptor_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &bindless_descriptor_set_layout;

    VkDescriptorSetVariableDescriptorCountAllocateInfo count_info {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO};
    uint32_t maxBinding = maxBindlessResource;
    count_info.descriptorSetCount = 1;
    count_info.pDescriptorCounts = &maxBinding;
    alloc_info.pNext = &count_info;

    if (vkAllocateDescriptorSets(device.device(),
                                 &alloc_info,
                                 &set ) != VK_SUCCESS)
    {
        throw std::runtime_error("M1k::ERR--------allocate bindless descriptor set failed");
    }
}

}
