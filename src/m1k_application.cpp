//
// Created by fangl on 2023/11/10.
//

#include "m1k_application.hpp"
#include "m1k_camera.hpp"
#include "m1k_buffer.hpp"
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

struct GlobalUbo {
    alignas(16) glm::mat4 projection_view{1.0f};
    alignas(16) glm::vec3 light_direction = glm::normalize(glm::vec3{1.0f, -3.0f, -1.0f});
};


M1kApplication::M1kApplication() {
    global_pool_ =
        M1kDescriptorPool::Builder(m1k_device_)
            .setMaxSets(M1kSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, M1kSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
    loadGameObjects();
}

M1kApplication::~M1kApplication() = default;

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
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .build();

    std::vector<VkDescriptorSet> global_descriptor_sets(M1kSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < global_descriptor_sets.size(); ++i) {
        auto buffer_info = ubo_buffers[i]->descriptorInfo();
        M1kDescriptorWriter(*global_set_layout, *global_pool_)
            .writeBuffer(0, &buffer_info)
            .build(global_descriptor_sets[i]);
    }

    SimpleRenderSystem simple_render_system{m1k_device_,
                                            m1k_renderer_.getSwapChainRenderPass(),
                                            global_set_layout->getDescriptorSetLayout()};

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
            int frame_index = m1k_renderer_.getFrameIndex();
            FrameInfo frame_info{
                frame_index,
                frame_time,
                command_buffer,
                camera,
                global_descriptor_sets[frame_index]
            };

            // update
            GlobalUbo ubo{};
            ubo.projection_view = camera.getProjection() * camera.getView();
            ubo_buffers[frame_index]->writeToBuffer(&ubo);
            ubo_buffers[frame_index]->flush();

            // WRONG: ???? WHY?????
            /*
             * [ VUID-VkMappedMemoryRange-size-01390 ] Object 0: handle = 0x3fbcd60000000028,
             * type = VK_OBJECT_TYPE_DEVICE_MEMORY;
             * | MessageID = 0xdd4e6d8b | vkFlushMappedMemoryRanges():
             * pMemoryRanges[0].size (80) is not a multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize (64)
             * and offset + size (0 + 80 = 80) not equal to the memory size (128).
             * The Vulkan spec states: If size is not equal to VK_WHOLE_SIZE,
             * size must either be a multiple of VkPhysicalDeviceLimits::nonCoherentAtomSize,
             * or offset plus size must equal the size of memory
             * (https://vulkan.lunarg.com/doc/view/1.3.268.0/windows/1.3-extensions/vkspec.html#VUID-VkMappedMemoryRange-size-01390)

             */
            // ubo_buffers[frame_index]->writeToIndex(&ubo, frame_index);
            // ubo_buffers[frame_index]->flushIndex(frame_index);

            // render
            m1k_renderer_.beginSwapChainRenderPass(command_buffer);
            simple_render_system.renderGameObjects(frame_info, game_objects_);
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
