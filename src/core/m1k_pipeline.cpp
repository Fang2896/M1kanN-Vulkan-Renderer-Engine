//
// Created by fangl on 2023/11/10.
//

#include "core/m1k_pipeline.hpp"
#include "m1k_model.hpp"

// std
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

namespace m1k {

M1kPipeline::M1kPipeline(M1kDevice& device,
            PipelineConfigInfo& config_info,
            const std::string& vert_filepath,
            const std::string& frag_filepath) : m1k_device_(device) {

          createGraphicPipeline(config_info, vert_filepath, frag_filepath);
}

M1kPipeline::~M1kPipeline() {
    vkDestroyShaderModule(m1k_device_.device(), vert_shader_module_, nullptr);
    vkDestroyShaderModule(m1k_device_.device(), frag_shader_module_, nullptr);
    vkDestroyPipeline(m1k_device_.device(), graphics_pipeline_, nullptr);
}

std::vector<char> M1kPipeline::readFile(const std::string& filepath) {
    std::ifstream file{filepath, std::ios::ate | std::ios::binary};

    if(!file.is_open()) {
        throw std::runtime_error("failed to open file : " + filepath);
    }

    size_t file_size = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    file.close();
    return buffer;
}

void M1kPipeline::bind(VkCommandBuffer command_buffer) {
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline_);

}

void M1kPipeline::createGraphicPipeline(PipelineConfigInfo& config_info,
                           const std::string& vert_filepath,
                           const std::string& frag_filepath) {
    assert(
        config_info.pipeline_layout != VK_NULL_HANDLE &&
        "Cannot create graphics pipeline :: No pipelineLayout provided in configInfo");
    assert(
        config_info.render_pass != VK_NULL_HANDLE &&
        "Cannot create graphics pipeline :: No render pass provided in configInfo");

    auto vert_code = readFile(vert_filepath);
    auto frag_code = readFile(frag_filepath);

    std::cout << "Vertex Shader Code Size : " << vert_code.size() << "\n";
    std::cout << "Fragment Shader Code Size : " << frag_code.size() << "\n";

    createShaderModule(vert_code, &vert_shader_module_);
    createShaderModule(frag_code, &frag_shader_module_);

    VkPipelineShaderStageCreateInfo shader_stages[2];

    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = vert_shader_module_;
    shader_stages[0].pName = "main";
    shader_stages[0].flags = 0;
    shader_stages[0].pNext = nullptr;
    shader_stages[0].pSpecializationInfo = nullptr;

    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = frag_shader_module_;
    shader_stages[1].pName = "main";
    shader_stages[1].flags = 0;
    shader_stages[1].pNext = nullptr;
    shader_stages[1].pSpecializationInfo = nullptr;

    auto& binding_descriptions = config_info.binding_descriptions;
    auto& attribute_descriptions = config_info.attribute_descriptions;

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
    vertex_input_info.vertexBindingDescriptionCount = static_cast<uint32_t>(binding_descriptions.size());
    vertex_input_info.pVertexAttributeDescriptions = attribute_descriptions.data();
    vertex_input_info.pVertexBindingDescriptions = binding_descriptions.data();

    // last
    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &config_info.input_assembly_info;
    pipeline_info.pViewportState = &config_info.viewport_info;
    pipeline_info.pRasterizationState = &config_info.rasterization_info;

    // set MSAA
    config_info.multisample_info.rasterizationSamples = m1k_device_.maxMSAASampleCount();
    config_info.multisample_info.sampleShadingEnable = VK_TRUE;
    config_info.multisample_info.minSampleShading = 0.4f;
    pipeline_info.pMultisampleState = &config_info.multisample_info;

    pipeline_info.pColorBlendState = &config_info.color_blend_info;
    pipeline_info.pDepthStencilState = &config_info.depth_stencil_info;
    pipeline_info.pDynamicState = &config_info.dynamic_state_info;

    pipeline_info.layout = config_info.pipeline_layout;
    pipeline_info.renderPass = config_info.render_pass;
    pipeline_info.subpass = config_info.subpass;

    pipeline_info.basePipelineIndex = -1;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    if(vkCreateGraphicsPipelines(m1k_device_.device(),
            VK_NULL_HANDLE,
            1,
            &pipeline_info,
            nullptr,
            &graphics_pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline");
    }
}

void M1kPipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shader_module) {
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if(vkCreateShaderModule(m1k_device_.device(), &create_info, nullptr, shader_module) != VK_SUCCESS) {
        throw std::runtime_error("fail to create shader module");
    }
}


// note the copy issue of pipeline config
void M1kPipeline::defaultPipelineConfigInfo(PipelineConfigInfo& config_info) {
    config_info.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    config_info.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config_info.input_assembly_info.primitiveRestartEnable = VK_FALSE;

    config_info.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    config_info.viewport_info.viewportCount = 1;
    config_info.viewport_info.pViewports = nullptr;
    config_info.viewport_info.scissorCount = 1;
    config_info.viewport_info.pScissors = nullptr;

    config_info.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    config_info.rasterization_info.depthClampEnable = VK_FALSE;
    config_info.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    config_info.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    config_info.rasterization_info.lineWidth = 1.0f;
    config_info.rasterization_info.cullMode = VK_CULL_MODE_NONE;
    config_info.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    config_info.rasterization_info.depthBiasEnable = VK_FALSE;
    config_info.rasterization_info.depthBiasConstantFactor = 0.0f;  // Optional
    config_info.rasterization_info.depthBiasClamp = 0.0f;           // Optional
    config_info.rasterization_info.depthBiasSlopeFactor = 0.0f;     // Optional

    config_info.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    config_info.multisample_info.sampleShadingEnable = VK_FALSE;
    config_info.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;  // default setting
    config_info.multisample_info.minSampleShading = 1.0f;           // Optional
    config_info.multisample_info.pSampleMask = nullptr;             // Optional
    config_info.multisample_info.alphaToCoverageEnable = VK_FALSE;  // Optional
    config_info.multisample_info.alphaToOneEnable = VK_FALSE;       // Optional

    config_info.color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
    config_info.color_blend_attachment.blendEnable = VK_FALSE;
    config_info.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    config_info.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    config_info.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    config_info.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    config_info.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    config_info.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

    config_info.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    config_info.color_blend_info.logicOpEnable = VK_FALSE;
    config_info.color_blend_info.logicOp = VK_LOGIC_OP_COPY;  // Optional
    config_info.color_blend_info.attachmentCount = 1;
    config_info.color_blend_info.pAttachments = &config_info.color_blend_attachment;
    config_info.color_blend_info.blendConstants[0] = 0.0f;  // Optional
    config_info.color_blend_info.blendConstants[1] = 0.0f;  // Optional
    config_info.color_blend_info.blendConstants[2] = 0.0f;  // Optional
    config_info.color_blend_info.blendConstants[3] = 0.0f;  // Optional

    config_info.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    config_info.depth_stencil_info.depthTestEnable = VK_TRUE;
    config_info.depth_stencil_info.depthWriteEnable = VK_TRUE;
    config_info.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
    config_info.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
    config_info.depth_stencil_info.minDepthBounds = 0.0f;  // Optional
    config_info.depth_stencil_info.maxDepthBounds = 1.0f;  // Optional
    config_info.depth_stencil_info.stencilTestEnable = VK_FALSE;
    config_info.depth_stencil_info.front = {};  // Optional
    config_info.depth_stencil_info.back = {};   // Optional

    // pipeline -> dynamic viewport/scissor
    config_info.dynamics_state_enables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    config_info.dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    config_info.dynamic_state_info.pDynamicStates = config_info.dynamics_state_enables.data();
    config_info.dynamic_state_info.dynamicStateCount =
        static_cast<uint32_t>(config_info.dynamics_state_enables.size());
    config_info.dynamic_state_info.flags = 0;

    config_info.binding_descriptions = M1kModel::Vertex::getBindingDescriptions();
    config_info.attribute_descriptions = M1kModel::Vertex::getAttributeDescriptions();
}

void M1kPipeline::enableAlphaBlending(PipelineConfigInfo& config_info) {
    config_info.color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
    config_info.color_blend_attachment.blendEnable = VK_TRUE;
    config_info.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;   // Optional
    config_info.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // Optional
    config_info.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    config_info.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    config_info.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    config_info.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
}


}
