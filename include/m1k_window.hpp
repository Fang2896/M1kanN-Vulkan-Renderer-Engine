//
// Created by fangl on 2023/11/10.
//

#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <string>

namespace m1k {

class M1kWindow {
   public:
    M1kWindow(int w, int h, std::string name);
    ~M1kWindow();

    // copy version delete
    M1kWindow(const M1kWindow&) = delete;
    M1kWindow &operator=(const M1kWindow&) = delete;

    bool shouldClose() { return glfwWindowShouldClose(window_); }
    VkExtent2D getExtent() { return {static_cast<uint32_t>(kWidth), static_cast<uint32_t>(kHeight)}; }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

   private:
    void initWindow();

    const int kWidth;
    const int kHeight;

    std::string window_name_;
    GLFWwindow * window_;
};

}
