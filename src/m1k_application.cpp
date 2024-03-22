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
#include "ImGuiFileDialog.h"

// std
#include <array>
#include <iostream>
#include <chrono>


namespace m1k {

M1kApplication::M1kApplication() {
    initImGUI();

    global_pool_ =
        M1kDescriptorPool::Builder(m1k_device_)
            .setMaxSets(kMaxGlobalPoolSetSize)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 50 * M1kSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,kMaxMaterialsNumber)
            .build();

    bindless_pool_ =
        M1kDescriptorPool::Builder(m1k_device_)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kMaxBindlessResources)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)
            .setMaxSets( 1 * kMaxBindlessResources)
            .build();
    // loadGameObjects();
}


M1kApplication::~M1kApplication() {
    ImGui_ImplVulkan_Shutdown();
}


void M1kApplication::run() {
    std::vector<std::shared_ptr<M1kBuffer>> global_ubo_buffers(
        M1kSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < global_ubo_buffers.size(); ++i) {
        global_ubo_buffers[i] = std::make_unique<M1kBuffer>(
            m1k_device_, sizeof(GlobalUbo), 1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        global_ubo_buffers[i]->map();
    }

    global_set_layout_ =
        M1kDescriptorSetLayout::Builder(m1k_device_)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
    pbr_set_layout_ =
        M1kDescriptorSetLayout::Builder(m1k_device_)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        VK_SHADER_STAGE_ALL_GRAPHICS)
//            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
//                        VK_SHADER_STAGE_FRAGMENT_BIT)   // diffuseTex
//            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
//                        VK_SHADER_STAGE_FRAGMENT_BIT)   // roughnessTex
//            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
//                        VK_SHADER_STAGE_FRAGMENT_BIT)   // occlusionTex
//            .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
//                        VK_SHADER_STAGE_FRAGMENT_BIT)   // emissiveTex
//            .addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
//                        VK_SHADER_STAGE_FRAGMENT_BIT)   // normalTex
            .build();
    bindless_set_layout_ =
        M1kDescriptorSetLayout::Builder(m1k_device_)
            .addBinding(kBindlessTextureBinding,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        kMaxBindlessResources)
            .build_for_bindless();


    // for all UBOs of each frame and textures
    // JUST FOR TEST
    M1kTexture test_texture{m1k_device_,
                            "../assets/textures/checkboard_texture.png"};
    auto& test_texture_image_info =
        test_texture.getDescriptorImageInfo();  // VkDescriptorImageinfo

    // for TEST render system ONLY
    // for all UBOs of each frame and textures
    std::vector<VkDescriptorSet> global_descriptor_sets(
        M1kSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < M1kSwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
        auto global_buffer_info = global_ubo_buffers[i]->descriptorInfo();
        M1kDescriptorWriter(*global_set_layout_, *global_pool_)
            .writeBuffer(0, &global_buffer_info)
            .writeImage(1, &test_texture_image_info)
            .build(global_descriptor_sets[i]);
    }

    // for Bindless render system
    VkDescriptorSet bindless_descriptor_set;
    M1kDescriptorPool::getBindlessDescriptorSet(m1k_device_,
                                                  bindless_pool_->getPool(),
                                                  bindless_set_layout_->getDescriptorSetLayout(),
                                                  bindless_descriptor_set,
                                                    kMaxBindlessResources);

    // systems init
    point_light_system_ = std::make_unique<PointLightSystem>(
        m1k_device_, m1k_renderer_.getSwapChainRenderPass(),
        global_set_layout_->getDescriptorSetLayout());

//    pbr_render_system_ = std::make_unique<PbrRenderSystem>(
//        m1k_device_, m1k_renderer_.getSwapChainRenderPass(),
//        global_set_layout_->getDescriptorSetLayout(),
//        pbr_set_layout_->getDescriptorSetLayout());

    bindless_pbr_render_system_ = std::make_unique<BindlessPbrRenderSystem>(
        m1k_device_, m1k_renderer_.getSwapChainRenderPass(),
        global_set_layout_->getDescriptorSetLayout(),
        pbr_set_layout_->getDescriptorSetLayout(),
        bindless_set_layout_->getDescriptorSetLayout());

    M1kCamera camera{};
    camera.setViewTarget(glm::vec3(-1.0f, -2.0f, -2.5f), glm::vec3(0.0f,0.0f,0.0f));
    auto viewer_object = M1kGameObject::createGameObject(GameObjectType::Camera);  // no model, no renderer (camera object)
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

        frame_time = glm::min(frame_time, kMaxFrameTime);

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
                bindless_descriptor_set,
                game_objects_
            };

            // update global UBO!
            GlobalUbo ubo{};
            ubo.projection_matrix = camera.getProjection();
            ubo.view_matrix = camera.getView();
            ubo.inverse_view_matrix = camera.getViewInverse();

            point_light_system_->update(frame_info, ubo);

            global_ubo_buffers[frame_index]->writeToBuffer(&ubo);
            global_ubo_buffers[frame_index]->flush();

            // imgui
            loopImGUI(frame_info);

            // render
            m1k_renderer_.beginSwapChainRenderPass(command_buffer);

            bindless_pbr_render_system_->updateBindlessTextures(frame_info);

            point_light_system_->render(frame_info);
            // pbr_render_system_->render(frame_info);
            bindless_pbr_render_system_->render(frame_info);

            // render ImGui draw data
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);

            m1k_renderer_.endSwapChainRenderPass(command_buffer);
            m1k_renderer_.endFrame();

            // update bindless textures

            m1k_renderer_.submitQueue();
        }
    }

    vkDeviceWaitIdle(m1k_device_.device());
}

void M1kApplication::initImGUI() {
    //1: create descriptor pool_ for IMGUI
    // the size of the pool_ is very oversize, but it's copied from imgui demo itself.
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

    // NOTE!!! if enable the MSAA， this MUST be the SAME as app!
    init_info.MSAASamples = m1k_device_.maxMSAASampleCount();

    ImGui_ImplVulkan_Init(&init_info, m1k_renderer_.getSwapChainRenderPass());

    //execute a gpu command to upload imgui font textures
    // upload
    VkCommandBuffer temp_imgui_command_buffer = m1k_device_.beginSingleTimeCommands();
    ImGui_ImplVulkan_CreateFontsTexture(temp_imgui_command_buffer);
    m1k_device_.endSingleTimeCommands(temp_imgui_command_buffer);

    //clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void M1kApplication::loopImGUI(FrameInfo& frame_info) {
    // init Imgui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui::NewFrame();

    if (is_displaying_test_scene_) {
        // point light intensity control
        ImGui::Begin("Point Light Control");
        float current_intensity =
            point_light_system_->getOnePointLightIntensity(frame_info);
        if (current_intensity >= 0 &&
            ImGui::SliderFloat("Point Light Intensity",
                               &current_intensity, 0.0f, 1.0f)) {
            point_light_system_->setAllPointLightsIntensity(
                current_intensity, frame_info);
        }

        ImGui::End();
    }

    // load model button
    if (ImGui::Button("Load Model")) {
        IGFD::FileDialogConfig config;
        config.path = default_model_select_path_;
        ImGuiFileDialog::Instance()->OpenDialog(
            "ChooseModelDialog", "Choose Model", ".gltf,.glb",
            config);
    }
    if (ImGuiFileDialog::Instance()->Display("ChooseModelDialog")) {
        std::string selected_file_path = "";
        if (ImGuiFileDialog::Instance()->IsOk()) {
            selected_file_path =
                ImGuiFileDialog::Instance()->GetFilePathName();
        }
        if (!selected_file_path.empty()) {
            loadGameObjects(selected_file_path);
        }

        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGui::Button("Load Test Scene")) {
        if (!is_displaying_test_scene_) {
            loadDefaultScene();
            is_displaying_test_scene_ = true;
        } else {
            std::cout << "M1K::WARN========Test Scene is Displaying!"
                      << std::endl;
        }
    }

    if (ImGui::Button("Clear Whole Scene")) {
        vkDeviceWaitIdle(m1k_device_.device());
        game_objects_.clear();
        is_displaying_test_scene_ = false;
        std::cout << "M1K::INFO~~~~~~~~Cleared ALL Scene." << std::endl;
    }

    ImGui::Render();  // finish imgui frame
}

void M1kApplication::loadGameObjects(const std::string& path,
                                     glm::vec3 pos, glm::vec3 scale) {
     if(path.empty()) {
        std::cout << "M1K::WARN========Please specify the model's path!" << std::endl;
        return;
     }

     GameObjectType game_object_type = GameObjectType::PbrObject;

    auto target_object = M1kGameObject::createGameObject(game_object_type);
    target_object.model = std::make_shared<M1kModel>(m1k_device_,
                                                     *pbr_set_layout_,
                                                     *global_pool_,
                                                     path);
    target_object.transform.translation = pos;
    target_object.transform.scale = scale;
    target_object.transform.rotation = glm::vec3(0,90,0);
    game_objects_.emplace(target_object.getId(), std::move(target_object));
    std::cout << "M1K::INFO~~~~~~~~Load game object, path: " << path << std::endl;
}

void M1kApplication::loadDefaultScene() {
//    // vase objects:
//    std::shared_ptr<M1kModel> flat_vase_model = M1kModel::createModelFromFile(m1k_device_, "../assets/models/obj/flat_vase.obj");
//    std::shared_ptr<M1kModel> smooth_vase_model = M1kModel::createModelFromFile(m1k_device_, "../assets/models/obj/smooth_vase.obj");
//
//    auto flat_vase = M1kGameObject::createGameObject();
//    flat_vase.model = flat_vase_model;
//    flat_vase.transform.translation = {-.5f, .5f, 0.f};
//    flat_vase.transform.scale = {3.f, 1.5f, 3.f};
//    game_objects_.emplace(flat_vase.getId() ,std::move(flat_vase));
//
//    auto smooth_vase = M1kGameObject::createGameObject();
//    smooth_vase.model = smooth_vase_model;
//    smooth_vase.transform.translation = {.5f, .5f, 0.f};
//    smooth_vase.transform.scale = {3.f, 1.5f, 3.f};
//    game_objects_.emplace(smooth_vase.getId(), std::move(smooth_vase));
//
//    // plane objects:
//    std::shared_ptr<M1kModel> plane_object_model = M1kModel::createModelFromFile(m1k_device_, "../assets/models/obj/quad.obj");
//    auto plane_object = M1kGameObject::createGameObject();
//    plane_object.model = plane_object_model;
//    plane_object.transform.translation = {0.0f, 0.5f, 0.0f};
//    plane_object.transform.scale = glm::vec3(3.0f, 1.0f, 3.0f);
//    game_objects_.emplace(plane_object.getId() ,std::move(plane_object));

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
