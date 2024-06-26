//
// Created by fangl on 2023/11/10.
//

#pragma once

#include "core/m1k_buffer.hpp"
#include "core/m1k_descriptor.hpp"
#include "core/m1k_device.hpp"
#include "core/m1k_renderer.hpp"
#include "objects/m1k_game_object.hpp"
#include "objects/m1k_texture.hpp"
#include "ui/m1k_camera.hpp"
#include "utils/m1k_utils.hpp"

// #include "systems/pbr_render_system.hpp"
#include "systems/point_light_system.hpp"
#include "systems/bindless_pbr_render_system.hpp"

#include "ui/keyboard_movement_controller.hpp"
#include "ui/m1k_window.hpp"

#include "m1k_config.hpp"

// std
#include <memory>

namespace m1k {

class M1kApplication {
   public:
    const int kWidth = kWindowWidth;
    const int kHeight = kWindowHeight;

    M1kApplication();
    ~M1kApplication();

    // copy version delete
    M1kApplication(const M1kApplication&) = delete;
    M1kApplication &operator=(const M1kApplication&) = delete;

    void loadGameObjects(const std::string& path = "",
                         glm::vec3 pos = {0.0f,0.0f,0.0f},
                         glm::vec3 scale = {1.0f,1.0f,1.0f});
    void loadDefaultScene();

    void run();

   private:
    void initImGUI();
    void loopImGUI(FrameInfo& frame_info);

    M1kWindow m1k_window_{kWidth, kHeight, "Hello Vulkan"};
    M1kDevice m1k_device_{m1k_window_};
    M1kRenderer m1k_renderer_{m1k_window_, m1k_device_};

    // 这个unique_ptr总觉得要暴雷，最好注意一下内存管理
    std::unique_ptr<M1kDescriptorSetLayout> global_set_layout_{};
    std::unique_ptr<M1kDescriptorSetLayout> pbr_set_layout_{};
    std::unique_ptr<M1kDescriptorSetLayout> bindless_set_layout_{};

    std::unique_ptr<M1kDescriptorPool> global_pool_{};
    std::unique_ptr<M1kDescriptorPool> bindless_pool_{};
    std::unique_ptr<M1kDescriptorPool> imgui_pool_{};

    M1kGameObject::Map game_objects_{};

    std::unique_ptr<PointLightSystem> point_light_system_;
    // std::unique_ptr<PbrRenderSystem> pbr_render_system_;
    std::unique_ptr<BindlessPbrRenderSystem> bindless_pbr_render_system_;

    // state information
    bool is_displaying_test_scene_ = false;
    const std::string default_model_select_path_ =
        "../assets/models/glTF";
};

}
