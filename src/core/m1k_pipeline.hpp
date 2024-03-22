//
// Created by fangl on 2023/11/10.
//

#pragma once

#include "m1k_device.hpp"
#include "m1k_config.hpp"

// std
#include <string>
#include <vector>
#include <stdexcept>


namespace m1k {

// share config between multiple pipeline
struct PipelineConfigInfo {
    PipelineConfigInfo() = default;
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

    std::vector<VkVertexInputBindingDescription> binding_descriptions{};
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions{};

    VkPipelineViewportStateCreateInfo viewport_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
    VkPipelineRasterizationStateCreateInfo rasterization_info;
    VkPipelineMultisampleStateCreateInfo multisample_info;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    VkPipelineColorBlendStateCreateInfo color_blend_info;
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info;

    std::vector<VkDynamicState> dynamics_state_enables;
    VkPipelineDynamicStateCreateInfo dynamic_state_info;

    VkPipelineLayout pipeline_layout = nullptr;
    VkRenderPass render_pass = nullptr;

    uint32_t subpass = 0;
};


class M1kPipeline {
   public:
    M1kPipeline() = default;
    M1kPipeline(M1kDevice& device,
                PipelineConfigInfo& config_info,
                const std::string& vert_filepath,
                const std::string& frag_filepath);

    ~M1kPipeline();

    M1kPipeline(const M1kPipeline&) = delete;
    M1kPipeline operator=(const M1kPipeline&) = delete;

    void bind(VkCommandBuffer command_buffer);

    static void defaultPipelineConfigInfo(PipelineConfigInfo& config_info);
    static void enableAlphaBlending(PipelineConfigInfo& config_info);

   private:
    static std::vector<char> readFile(const std::string& filepath);

    void createGraphicPipeline(PipelineConfigInfo& config_info,
                               const std::string& vert_filepath,
                               const std::string& frag_filepath);

    void processPipelineCache(const std::string& cache_path);
    void savePipelineCache(const std::string& cache_path);

    void createShaderModule(const std::vector<char>& code, VkShaderModule* shader_module);

    M1kDevice &m1k_device_;
    VkPipeline graphics_pipeline_;
    VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;

    VkShaderModule vert_shader_module_;
    VkShaderModule frag_shader_module_;
};

}
