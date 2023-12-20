//
// Created by fangl on 2023/11/10.
//

#include "m1k_application.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

// std
#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>


namespace m1k {

#define MAX_FRAME_TIME 0.5f
#define MAX_MATERIALS_NUMBER 30

M1kApplication::M1kApplication() {
    initImGUI();

    global_pool_ =
        M1kDescriptorPool::Builder(m1k_device_)
            .setMaxSets(M1kSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, M1kSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,MAX_MATERIALS_NUMBER)
            .build();

    loadGameObjects();
}

M1kApplication::~M1kApplication() {
    ImGui_ImplVulkan_Shutdown();
}

void M1kApplication::run() {
    std::vector<std::unique_ptr<M1kBuffer>> ubo_buffers(M1kSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < ubo_buffers.size(); ++i) {
        ubo_buffers[i] = std::make_unique<M1kBuffer>(
            m1k_device_,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        ubo_buffers[i]->map();
    }

    auto global_set_layout = M1kDescriptorSetLayout::Builder(m1k_device_)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    // for all UBOs of each frame and textures
    M1kTexture test_texture{m1k_device_, "../assets/textures/test_texture.jpg"};
    auto& test_texture_image_info = test_texture.getDescriptorImageInfo();
    std::vector<VkDescriptorSet> global_descriptor_sets(M1kSwapChain::MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < M1kSwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto buffer_info = ubo_buffers[i]->descriptorInfo();
        M1kDescriptorWriter(*global_set_layout, *global_pool_)
            .writeBuffer(0, &buffer_info)
            .writeImage(1, &test_texture_image_info)
            .build(global_descriptor_sets[i]);
    }


    // systems init
    SimpleRenderSystem simple_render_system{m1k_device_,
                                            m1k_renderer_.getSwapChainRenderPass(),
                                            global_set_layout->getDescriptorSetLayout()};

    PointLightSystem point_light_system{m1k_device_,
                                            m1k_renderer_.getSwapChainRenderPass(),
                                            global_set_layout->getDescriptorSetLayout()};

    M1kCamera camera{};
    camera.setViewTarget(glm::vec3(-1.0f, -2.0f, -2.5f), glm::vec3(0.0f,0.0f,0.0f));

    auto viewer_object = M1kGameObject::createGameObject();  // no model, no renderer (camera object)
    viewer_object.transform.translation.z = -2.5f;
    KeyboardMovementController camera_controller{};

    //  game loop
    auto current_time = std::chrono::high_resolution_clock::now();
    while(!m1k_window_.shouldClose()) {
        glfwPollEvents();   // may block
        ImGui_ImplGlfw_NewFrame();

        auto new_time = std::chrono::high_resolution_clock::now();
        float frame_time =
            std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
        current_time = new_time;

        frame_time = glm::min(frame_time, MAX_FRAME_TIME);

        camera_controller.moveInPlaneXZ(m1k_window_.getGLFWwindow(), frame_time, viewer_object);
        camera.setViewYXZ(viewer_object.transform.translation, viewer_object.transform.rotation);

        float aspect = m1k_renderer_.getAspectRatio();
        // camera.setOrthographicProjection(-aspect,aspect,-1,1,-1,1);
        camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 200.0f);

        if(auto command_buffer = m1k_renderer_.beginFrame()) {
            int frame_index = m1k_renderer_.getFrameIndex();
            FrameInfo frame_info{
                frame_index,
                frame_time,
                command_buffer,
                camera,
                global_descriptor_sets[frame_index],
                game_objects_
            };

            // update
            GlobalUbo ubo{};
            ubo.projection_matrix = camera.getProjection();
            ubo.view_matrix = camera.getView();
            ubo.inverse_view_matrix = camera.getViewInverse();

            point_light_system.update(frame_info, ubo);

            ubo_buffers[frame_index]->writeToBuffer(&ubo);
            ubo_buffers[frame_index]->flush();

            // init Imgui frame
            ImGui_ImplVulkan_NewFrame();
            ImGui::NewFrame();

            // point light control
            ImGui::Begin("Point Light Control");
            float current_intensity = point_light_system.getOnePointLightIntensity(frame_info);
            if (current_intensity >= 0 && ImGui::SliderFloat("Point Light Intensity", &current_intensity, 0.0f, 1.0f)) {
                point_light_system.setAllPointLightsIntensity(current_intensity, frame_info);
            }
            ImGui::End();
            ImGui::Render();    // finish imgui frame

            // render
            m1k_renderer_.beginSwapChainRenderPass(command_buffer);

            simple_render_system.renderGameObjects(frame_info);
            point_light_system.render(frame_info);

            // render ImGui draw data
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);

            m1k_renderer_.endSwapChainRenderPass(command_buffer);
            m1k_renderer_.endFrame();
        }
    }

    vkDeviceWaitIdle(m1k_device_.device());
}

void M1kApplication::initImGUI() {
    //1: create descriptor pool for IMGUI
    // the size of the pool is very oversize, but it's copied from imgui demo itself.
    imgui_pool_ =
        M1kDescriptorPool::Builder(m1k_device_)
            .setMaxSets(M1kSwapChain::MAX_FRAMES_IN_FLIGHT * 2)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 20)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 20)
            .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 20)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 20)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 20)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 20)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 20)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 20)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 20)
            .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 20)
            .build();

    // 2: initialize imgui library

    //this initializes the core structures of imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    //this initializes imgui for SDL
    ImGui_ImplGlfw_InitForVulkan(m1k_window_.getGLFWwindow(), true);

    //this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m1k_device_.getVkInstance();
    init_info.PhysicalDevice = m1k_device_.getPhyDevice();
    init_info.Device = m1k_device_.device();
    init_info.Queue = m1k_device_.graphicsQueue();
    init_info.DescriptorPool = imgui_pool_->getPool();
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info, m1k_renderer_.getSwapChainRenderPass());

    //execute a gpu command to upload imgui font textures
    // upload
    VkCommandBuffer temp_imgui_command_buffer = m1k_device_.beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(temp_imgui_command_buffer);
    m1k_device_.endSingleTimeCommands(temp_imgui_command_buffer);

    //clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void M1kApplication::loadGameObjects() {
    // vase objects:
    std::shared_ptr<M1kModel> flat_vase_model = M1kModel::createModelFromFile(m1k_device_, "../assets/models/flat_vase.obj");
    std::shared_ptr<M1kModel> smooth_vase_model = M1kModel::createModelFromFile(m1k_device_, "../assets/models/smooth_vase.obj");

    auto flat_vase = M1kGameObject::createGameObject();
    flat_vase.model = flat_vase_model;
    flat_vase.transform.translation = {-.5f, .5f, 0.f};
    flat_vase.transform.scale = {3.f, 1.5f, 3.f};
    game_objects_.emplace(flat_vase.getId() ,std::move(flat_vase));

    auto smooth_vase = M1kGameObject::createGameObject();
    smooth_vase.model = smooth_vase_model;
    smooth_vase.transform.translation = {.5f, .5f, 0.f};
    smooth_vase.transform.scale = {3.f, 1.5f, 3.f};
    game_objects_.emplace(smooth_vase.getId(), std::move(smooth_vase));

    // plane objects:
    std::shared_ptr<M1kModel> plane_object_model = M1kModel::createModelFromFile(m1k_device_, "../assets/models/quad.obj");
    auto plane_object = M1kGameObject::createGameObject();
    plane_object.model = plane_object_model;
    plane_object.transform.translation = {0.0f, 0.5f, 0.0f};
    plane_object.transform.scale = glm::vec3(3.0f, 1.0f, 3.0f);
    game_objects_.emplace(plane_object.getId() ,std::move(plane_object));

    // point light
    std::vector<glm::vec3> point_light_colors{
        {1.f, .1f, .1f},
        {.1f, .1f, 1.f},
        {.1f, 1.f, .1f},
        {1.f, 1.f, .1f},
        {.1f, 1.f, 1.f},
        {1.f, 1.f, 1.f}  //
    };

    for (int i = 0; i < point_light_colors.size(); ++i) {
        auto point_light = M1kGameObject::makePointLight(0.8f);
        point_light.color = point_light_colors[i];
        auto rotate_light = glm::rotate(
            glm::mat4(1.0f),
            (i * glm::two_pi<float>()) / point_light_colors.size(),
            {0.f, -1.f, 0.f});

        point_light.transform.translation = glm::vec3(rotate_light * glm::vec4(-1.f, -1.f, -1.f, 1.f));
        game_objects_.emplace(point_light.getId(), std::move(point_light));
    }
}


}
