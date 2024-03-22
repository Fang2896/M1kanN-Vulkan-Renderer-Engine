//
// Created by fangl on 2023/12/1.
//

#pragma once

#include "m1k_data_struct.hpp"
#include "m1k_game_object.hpp"
#include "ui/m1k_camera.hpp"

// lib
#include <vulkan/vulkan.h>


namespace m1k {

struct FrameInfo {
    int frame_index;
    float frame_time;
    VkCommandBuffer command_buffer;
    M1kCamera &camera;
    VkDescriptorSet global_descriptor_set;
    VkDescriptorSet bindless_descriptor_set;
    M1kGameObject::Map &game_objects;
};


}
