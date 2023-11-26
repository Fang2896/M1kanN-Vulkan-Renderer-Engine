//
// Created by fangl on 2023/11/10.
//

#include "m1k_application.hpp"
#include "m1k_camera.hpp"
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
    M1kCamera camera{};

    SimpleRenderSystem simple_render_system{m1k_device_, m1k_renderer_.getSwapChainRenderPass()};

    while(!m1k_window_.shouldClose()) {
        glfwPollEvents();

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

// temporary helper function, creates a 1x1x1 cube centered at offset
std::unique_ptr<M1kModel> createCubeModel(M1kDevice& device, glm::vec3 offset) {
    std::vector<M1kModel::Vertex> vertices{
        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
    };

    for (auto& v : vertices) {
        v.position += offset;
    }

    return std::make_unique<M1kModel>(device, vertices);
}

void M1kApplication::loadGameObjects() {
    std::shared_ptr<M1kModel> cube_model = createCubeModel(m1k_device_, {0.0f,0.0f,0.0f});

    auto cube = M1kGameObject::createGameObject();
    cube.model = cube_model;
    cube.transform.translation = {0.0f, 0.0f, 2.5f};
    cube.transform.scale = {0.5f, 0.5f, 0.5f};

    game_objects_.push_back(std::move(cube));
}


}
