//
// Created by fangl on 2024/02/26.
//

#include "bindless_pbr_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <iostream>


namespace m1k {

BindlessPbrRenderSystem::BindlessPbrRenderSystem(M1kDevice &device, VkRenderPass render_pass,
                                 VkDescriptorSetLayout global_set_layout,
                                 VkDescriptorSetLayout pbr_set_layout,
                                 VkDescriptorSetLayout bindless_set_layout)
    : m1k_device_(device), global_set_layout_(global_set_layout),
      pbr_set_layout_(pbr_set_layout), bindless_set_layout_(bindless_set_layout)
{
    createPipelineLayout();
    createPipeline(render_pass);
}

BindlessPbrRenderSystem::~BindlessPbrRenderSystem() {
    vkDestroyPipelineLayout(m1k_device_.device(), pipeline_layout_, nullptr);
}

void BindlessPbrRenderSystem::createPipelineLayout() {
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts{
        global_set_layout_, bindless_set_layout_, pbr_set_layout_};

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
    pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();

    if(vkCreatePipelineLayout(m1k_device_.device(),
                               &pipeline_layout_info,
                               nullptr,
                               &pipeline_layout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
    }
}

void BindlessPbrRenderSystem::createPipeline(VkRenderPass render_pass) {
    assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipeline_config{};
    M1kPipeline::defaultPipelineConfigInfo(pipeline_config);
    pipeline_config.render_pass = render_pass;
    pipeline_config.pipeline_layout = pipeline_layout_;
    m1k_pipeline_ = std::make_unique<M1kPipeline>(
        m1k_device_,
        pipeline_config,
        "./shaders/binaries/bindless_pbr_shader.vert.spv",
        "./shaders/binaries/bindless_pbr_shader.frag.spv");
}

void BindlessPbrRenderSystem::render(FrameInfo &frame_info) {
    m1k_pipeline_->bind(frame_info.command_buffer);

    // bind global descriptor set
    vkCmdBindDescriptorSets(
        frame_info.command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout_,
        0, 1,
        &frame_info.global_descriptor_set,
        0, nullptr);

    for(auto& kv : frame_info.game_objects) {
        auto &obj = kv.second;

        // filter
        if(obj.getType() != GameObjectType::PbrObject) continue;

        obj.model->draw(frame_info.command_buffer,
                        frame_info.bindless_descriptor_set,
                        pipeline_layout_);
    }
}

void BindlessPbrRenderSystem::updateBindlessTextures(m1k::FrameInfo& frame_info) {
    VkWriteDescriptorSet bindless_descriptor_writes[kMaxBindlessResources];
    VkDescriptorImageInfo bindless_image_info[kMaxBindlessResources];
    uint32_t current_write_index = 0;

    for(auto& pair : frame_info.game_objects) {
        if(pair.second.getType() != GameObjectType::PbrObject) continue;

        auto& model = pair.second.model;
        auto& to_be_updated_textures = model->to_update_textures_;

        for(auto& p : to_be_updated_textures) {
            auto& texture = p.second;
            VkWriteDescriptorSet& descriptor_write = bindless_descriptor_writes[current_write_index];
            descriptor_write = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
            descriptor_write.descriptorCount = 1;
            descriptor_write.dstArrayElement = texture->getIndex();
            descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_write.dstSet = frame_info.bindless_descriptor_set;
            descriptor_write.dstBinding = kBindlessTextureBinding;

            auto sampler = texture->getDescriptorImageInfo().sampler;
            VkDescriptorImageInfo& descriptor_image_info = bindless_image_info[current_write_index];
            descriptor_image_info.sampler = sampler;
            descriptor_image_info.imageView = texture->getDescriptorImageInfo().imageView;
            descriptor_image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            descriptor_write.pImageInfo = &descriptor_image_info;

            ++current_write_index;
        }

        to_be_updated_textures.clear();
    }

    if (current_write_index) {
        vkUpdateDescriptorSets(m1k_device_.device(),
                               current_write_index,
                               bindless_descriptor_writes,
                               0,
                               nullptr);
    }
}

}

