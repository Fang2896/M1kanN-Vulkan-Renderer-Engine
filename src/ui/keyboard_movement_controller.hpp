//
// Created by fangl on 2023/11/27.
//

#pragma once

#include "../m1k_game_object.hpp"
#include "m1k_window.hpp"

namespace m1k {

class KeyboardMovementController {
public:
    struct KeyMappings {
        int move_left = GLFW_KEY_A;
        int move_right = GLFW_KEY_D;
        int move_forward = GLFW_KEY_W;
        int move_backward = GLFW_KEY_S;
        int move_up = GLFW_KEY_E;
        int move_down = GLFW_KEY_Q;
        int look_left = GLFW_KEY_LEFT;
        int look_right = GLFW_KEY_RIGHT;
        int look_up = GLFW_KEY_UP;
        int look_down = GLFW_KEY_DOWN;
    };

    void moveInPlaneXZ(GLFWwindow *window, float dt, M1kGameObject &game_object);

    KeyMappings keys{};
    float move_speed{3.0f};
    float look_speed{1.5f};
};


}
