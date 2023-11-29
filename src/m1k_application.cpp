//
// Created by fangl on 2023/11/10.
//

#include "m1k_application.hpp"
#include "m1k_camera.hpp"
#include "simple_render_system.hpp"
#include "keyboard_movement_controller.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <iostream>
#include <chrono>


namespace m1k {
#define MAX_FRAME_TIME 0.5f

M1kApplication::M1kApplication() {
    loadGameObjects();
}

M1kApplication::~M1kApplication() = default;

void M1kApplication::run() {
    SimpleRenderSystem simple_render_system{m1k_device_, m1k_renderer_.getSwapChainRenderPass()};

    std::cout << "max push constant size: " << m1k_device_.properties.limits.maxPushConstantsSize << "\n";
    M1kCamera camera{};
    camera.setViewTarget(glm::vec3(-1.0f, -2.0f, -2.5f), glm::vec3(0.0f,0.0f,2.5f));

    auto viewer_object = M1kGameObject::createGameObject();  // no model, no renderer (camera object)
    KeyboardMovementController camera_controller{};

    auto current_time = std::chrono::high_resolution_clock::now();
    while(!m1k_window_.shouldClose()) {
        glfwPollEvents();   // may block

        auto new_time = std::chrono::high_resolution_clock::now();
        float frame_time =
            std::chrono::duration<float, std::chrono::seconds::period>(new_time - current_time).count();
        current_time = new_time;

        frame_time = glm::min(frame_time, MAX_FRAME_TIME);

        camera_controller.moveInPlaneXZ(m1k_window_.getGLFWwindow(), frame_time, viewer_object);
        camera.setViewYXZ(viewer_object.transform.translation, viewer_object.transform.rotation);

        float aspect = m1k_renderer_.getAspectRatio();
        // camera.setOrthographicProjection(-aspect,aspect,-1,1,-1,1);
        camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);

        if(auto command_buffer = m1k_renderer_.beginFrame()) {
            m1k_renderer_.beginSwapChainRenderPass(command_buffer);
            simple_render_system.renderGameObjects(command_buffer, game_objects_, camera);
            m1k_renderer_.endSwapChainRenderPass(command_buffer);
            m1k_renderer_.endFrame();
        }
    }
    vkDeviceWaitIdle(m1k_device_.device());
}

void M1kApplication::loadGameObjects() {
    std::shared_ptr<M1kModel> cube_model = M1kModel::createModelFromFile(m1k_device_, "../resources/models/flat_vase.obj");

    auto cube = M1kGameObject::createGameObject();
    cube.model = cube_model;
    cube.transform.translation = {0.0f, 0.5f, 2.5f};
    cube.transform.scale = glm::vec3(3.0f);

    game_objects_.push_back(std::move(cube));
}


}
