//
// Created by fangl on 2023/11/10.
//

#include "m1k_application.hpp"
#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <array>
#include <iostream>


namespace m1k {

M1kApplication::M1kApplication() {
    loadGameObjects();
}

M1kApplication::~M1kApplication() = default;

void M1kApplication::run() {
    std::cout << "max push constant size: " << m1k_device_.properties.limits.maxPushConstantsSize << "\n";

    SimpleRenderSystem simple_render_system{m1k_device_, m1k_renderer.getSwapChainRenderPass()};

    while(!m1k_window_.shouldClose()) {
        glfwPollEvents();

        if(auto command_buffer = m1k_renderer.beginFrame()) {
            m1k_renderer.beginSwapChainRenderPass(command_buffer);
            simple_render_system.renderGameObjects(command_buffer, game_objects_);
            m1k_renderer.endSwapChainRenderPass(command_buffer);
            m1k_renderer.endFrame();
        }
    }

    vkDeviceWaitIdle(m1k_device_.device());
}

void M1kApplication::loadGameObjects() {
    std::vector<M1kModel::Vertex> vertices {
        {{0.0f, -0.5f}, {1,0,0}},
        {{0.5f, 0.5f}, {0,1,0}},
        {{-0.5f, 0.5f}, {0,0,1}}
    };

    auto m1K_model = std::make_shared<M1kModel>(m1k_device_, vertices);

    auto triangle = M1kGameObject::createGameObject();
    triangle.model = m1K_model;
    triangle.color = {.1f, .8f, .1f};
    triangle.transform_2d_component.translation.x = .2f;
    triangle.transform_2d_component.scale = {2.0f, 0.5f};
    triangle.transform_2d_component.rotation = 0.25f * glm::two_pi<float>();

    game_objects_.push_back(std::move(triangle));
}

}
