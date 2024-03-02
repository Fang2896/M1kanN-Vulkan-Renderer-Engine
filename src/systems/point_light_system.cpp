//
// Created by fangl on 2023/12/2.
//

#include "point_light_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <iostream>
#include <map>


namespace m1k {

struct PointLightPushConstants {
    glm::vec4 position{};
    glm::vec4 color{};
    float radius;
};

PointLightSystem::PointLightSystem(M1kDevice &device, VkRenderPass render_pass, VkDescriptorSetLayout global_set_layout)
    : m1k_device_(device) {
    createPipelineLayout(global_set_layout);
    createPipeline(render_pass);
}

PointLightSystem::~PointLightSystem() {
    vkDestroyPipelineLayout(m1k_device_.device(), pipeline_layout_, nullptr);
}

void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout global_set_layout) {

    VkPushConstantRange push_constant_range{};
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(PointLightPushConstants);

    std::vector<VkDescriptorSetLayout> descriptor_set_layouts{global_set_layout};

    VkPipelineLayoutCreateInfo pipeline_layout_info{};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
    pipeline_layout_info.pSetLayouts = descriptor_set_layouts.data();
    pipeline_layout_info.pushConstantRangeCount = 1;
    pipeline_layout_info.pPushConstantRanges = &push_constant_range;

    if(vkCreatePipelineLayout(m1k_device_.device(),
                              &pipeline_layout_info,
                              nullptr,
                              &pipeline_layout_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
    }
}

void PointLightSystem::createPipeline(VkRenderPass render_pass) {
    assert(pipeline_layout_ != nullptr && "Cannot create pipeline before pipeline layout");

    PipelineConfigInfo pipeline_config{};
    M1kPipeline::defaultPipelineConfigInfo(pipeline_config);
    M1kPipeline::enableAlphaBlending(pipeline_config);  // blending

    pipeline_config.binding_descriptions.clear();
    pipeline_config.attribute_descriptions.clear();

    pipeline_config.render_pass = render_pass;
    pipeline_config.pipeline_layout = pipeline_layout_;
    m1k_pipeline_ = std::make_unique<M1kPipeline>(
        m1k_device_,
        pipeline_config,
        "./shaders/binaries/point_light.vert.spv",
        "./shaders/binaries/point_light.frag.spv");
}

void PointLightSystem::update(FrameInfo &frame_info, GlobalUbo &ubo) {
    auto rotate_light = glm::rotate(
        glm::mat4(1.0f),
        frame_info.frame_time,
        {0.f, -1.f, 0.f});

    int light_index = 0;
    for(auto &kv : frame_info.game_objects) {
        auto &obj = kv.second;
        // filter
        if(obj.point_light == nullptr)  continue;

        assert(light_index < MAX_LIGHTS && "Point lights exceed maximum specified.");

        // update light position
        obj.transform.translation = glm::vec3(rotate_light * glm::vec4(obj.transform.translation, 1.f));

        // copy light to ubo
        ubo.point_lights[light_index].position = glm::vec4(obj.transform.translation, 1.0f);
        ubo.point_lights[light_index].color = glm::vec4(obj.color, obj.point_light->light_intensity);

        light_index++;
    }

    ubo.num_lights = light_index;
}

void PointLightSystem::render(FrameInfo &frame_info) {
    // sort lights
    std::map<float, M1kGameObject::id_t> sorted;
    for(auto& kv : frame_info.game_objects) {
        auto& obj = kv.second;
        if(obj.getType() != GameObjectType::PointLight) continue;

        auto offset = frame_info.camera.getPosition() - obj.transform.translation;
        float dist_squared = glm::dot(offset, offset);
        sorted[dist_squared] = obj.getId();
    }

    m1k_pipeline_->bind(frame_info.command_buffer);

    vkCmdBindDescriptorSets(
        frame_info.command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout_,
        0, 1,
        &frame_info.global_descriptor_set,
        0, nullptr);

    // render in point light distance order
    for(auto it = sorted.rbegin(); it != sorted.rend(); it++) {
        auto &obj = frame_info.game_objects.at(it->second);

        PointLightPushConstants push{};
        push.position = glm::vec4(obj.transform.translation, 1.0f);
        push.color = glm::vec4(obj.color, obj.point_light->light_intensity);
        push.radius = obj.transform.scale.x;

        vkCmdPushConstants(
              frame_info.command_buffer,
              pipeline_layout_,
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
              0, sizeof(PointLightPushConstants),
              &push
        );

        vkCmdDraw(frame_info.command_buffer, 6, 1, 0, 0);
    }
}

void PointLightSystem::setAllPointLightsIntensity(float intensity, FrameInfo &frame_info) {
    for(auto& kv : frame_info.game_objects) {
        auto &obj = kv.second;
        if(obj.point_light == nullptr) continue;

        obj.point_light->light_intensity = intensity;
    }
}

void PointLightSystem::setSinglePointLightIntensity(uint32_t id, float intensity, FrameInfo &frame_info) {
    auto &obj = frame_info.game_objects.at(id);
    obj.point_light->light_intensity = intensity;
}

float PointLightSystem::getOnePointLightIntensity(FrameInfo &frame_info) {
    for(auto& kv : frame_info.game_objects) {
        auto& obj = kv.second;
        if(obj.point_light != nullptr) return obj.point_light->light_intensity;
    }

    return -1.0f;
}

}


