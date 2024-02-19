//
// Created by fangl on 2023/11/10.
//

#pragma once

#include "core/m1k_buffer.hpp"
#include "core/m1k_descriptor.hpp"
#include "core/m1k_device.hpp"
#include "core/m1k_renderer.hpp"
#include "m1k_camera.hpp"
#include "m1k_game_object.hpp"
#include "m1k_texture.hpp"
#include "systems/point_light_system.hpp"
#include "systems/simple_render_system.hpp"
#include "ui/keyboard_movement_controller.hpp"
#include "ui/m1k_window.hpp"

// std
#include <memory>

namespace m1k {

class M1kApplication {
   public:
    static constexpr int kWidth = 1366;
    static constexpr int kHeight = 768;

    M1kApplication();
    ~M1kApplication();

    // copy version delete
    M1kApplication(const M1kApplication&) = delete;
    M1kApplication &operator=(const M1kApplication&) = delete;

    void run();

   private:
    void initImGUI();
    void loadGameObjects();

    M1kWindow m1k_window_{kWidth, kHeight, "Hello Vulkan"};
    M1kDevice m1k_device_{m1k_window_};
    M1kRenderer m1k_renderer_{m1k_window_, m1k_device_};

    std::unique_ptr<M1kDescriptorPool> global_pool_{};
    std::unique_ptr<M1kDescriptorPool> imgui_pool_{};

    M1kGameObject::Map game_objects_;
};

}
