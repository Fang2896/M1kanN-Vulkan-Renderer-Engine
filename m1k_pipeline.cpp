//
// Created by fangl on 2023/11/10.
//

#include "m1k_pipeline.hpp"

// std
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

namespace m1k {

M1kPipeline::M1kPipeline(M1kDevice& device,
            const PipelineConfigInfo& config_info,
            const std::string& vert_filepath,
            const std::string& frag_filepath) : m1kDevice_(device) {
          createGraphicPipeline(config_info, vert_filepath, frag_filepath);
}

M1kPipeline::~M1kPipeline() {
    vkDestroyShaderModule(m1kDevice_.device(), vert_shader_module_, nullptr);
    vkDestroyShaderModule(m1kDevice_.device(), frag_shader_module_, nullptr);
    vkDestroyPipeline(m1kDevice_.device(), graphics_pipeline_, nullptr);
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

PipelineConfigInfo M1kPipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
    PipelineConfigInfo configInfo{};

    configInfo.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    configInfo.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    configInfo.input_assembly_info.primitiveRestartEnable = VK_FALSE;
 
    configInfo.viewport.x = 0.0f;
    configInfo.viewport.y = 0.0f;
    configInfo.viewport.width = static_cast<float>(width);
    configInfo.viewport.height = static_cast<float>(height);
    configInfo.viewport.minDepth = 0.0f;
    configInfo.viewport.maxDepth = 1.0f;
 
    configInfo.scissor.offset = {0, 0};
    configInfo.scissor.extent = {width, height};
 
    configInfo.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    configInfo.viewport_info.viewportCount = 1;
    configInfo.viewport_info.pViewports = &configInfo.viewport;
    configInfo.viewport_info.scissorCount = 1;
    configInfo.viewport_info.pScissors = &configInfo.scissor;
 
    configInfo.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    configInfo.rasterization_info.depthClampEnable = VK_FALSE;
    configInfo.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    configInfo.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    configInfo.rasterization_info.lineWidth = 1.0f;
    configInfo.rasterization_info.cullMode = VK_CULL_MODE_NONE;
    configInfo.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    configInfo.rasterization_info.depthBiasEnable = VK_FALSE;
    configInfo.rasterization_info.depthBiasConstantFactor = 0.0f;  // Optional
    configInfo.rasterization_info.depthBiasClamp = 0.0f;           // Optional
    configInfo.rasterization_info.depthBiasSlopeFactor = 0.0f;     // Optional
 
    configInfo.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    configInfo.multisample_info.sampleShadingEnable = VK_FALSE;
    configInfo.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    configInfo.multisample_info.minSampleShading = 1.0f;           // Optional
    configInfo.multisample_info.pSampleMask = nullptr;             // Optional
    configInfo.multisample_info.alphaToCoverageEnable = VK_FALSE;  // Optional
    configInfo.multisample_info.alphaToOneEnable = VK_FALSE;       // Optional
 
    configInfo.color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    configInfo.color_blend_attachment.blendEnable = VK_FALSE;
    configInfo.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    configInfo.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    configInfo.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    configInfo.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    configInfo.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    configInfo.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional
 
    configInfo.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    configInfo.color_blend_info.logicOpEnable = VK_FALSE;
    configInfo.color_blend_info.logicOp = VK_LOGIC_OP_COPY;  // Optional
    configInfo.color_blend_info.attachmentCount = 1;
    configInfo.color_blend_info.pAttachments = &configInfo.color_blend_attachment;
    configInfo.color_blend_info.blendConstants[0] = 0.0f;  // Optional
    configInfo.color_blend_info.blendConstants[1] = 0.0f;  // Optional
    configInfo.color_blend_info.blendConstants[2] = 0.0f;  // Optional
    configInfo.color_blend_info.blendConstants[3] = 0.0f;  // Optional
 
    configInfo.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    configInfo.depth_stencil_info.depthTestEnable = VK_TRUE;
    configInfo.depth_stencil_info.depthWriteEnable = VK_TRUE;
    configInfo.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
    configInfo.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
    configInfo.depth_stencil_info.minDepthBounds = 0.0f;  // Optional
    configInfo.depth_stencil_info.maxDepthBounds = 1.0f;  // Optional
    configInfo.depth_stencil_info.stencilTestEnable = VK_FALSE;
    configInfo.depth_stencil_info.front = {};  // Optional
    configInfo.depth_stencil_info.back = {};   // Optional

    return configInfo;
}

void M1kPipeline::createGraphicPipeline(const PipelineConfigInfo& config_info,
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

    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = nullptr;
    vertex_input_info.pVertexBindingDescriptions = nullptr;

    // last
    VkGraphicsPipelineCreateInfo pipeline_info{};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &config_info.input_assembly_info;
    pipeline_info.pViewportState = &config_info.viewport_info;
    pipeline_info.pRasterizationState = &config_info.rasterization_info;
    pipeline_info.pMultisampleState = &config_info.multisample_info;
    pipeline_info.pColorBlendState = &config_info.color_blend_info;
    pipeline_info.pDepthStencilState = &config_info.depth_stencil_info;
    pipeline_info.pDynamicState = nullptr;

    pipeline_info.layout = config_info.pipeline_layout;
    pipeline_info.renderPass = config_info.render_pass;
    pipeline_info.subpass = config_info.subpass;

    pipeline_info.basePipelineIndex = -1;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    if(vkCreateGraphicsPipelines(
            m1kDevice_.device(),
            VK_NULL_HANDLE,
            1,
            &pipeline_info,
            nullptr,
            &graphics_pipeline_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline");
    }
}

void M1kPipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shader_module) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if(vkCreateShaderModule(m1kDevice_.device(), &createInfo, nullptr, shader_module) != VK_SUCCESS) {
        throw std::runtime_error("fail to create shader module");
    }

}

}
