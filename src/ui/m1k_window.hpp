//
// Created by fangl on 2023/11/10.
//

#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <vulkan/vulkan.h>

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
    VkExtent2D getExtent() { return {static_cast<uint32_t>(width_), static_cast<uint32_t>(height_)}; }
    bool wasWindowResized() { return framebuffer_resized_; }
    void resetWindowResizedFlag() { framebuffer_resized_ = false; }
    GLFWwindow *getGLFWwindow() const { return window_; }

    void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);

   private:
    static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
    void initWindow();

    int width_;
    int height_;
    bool framebuffer_resized_ = false;

    std::string window_name_;
    GLFWwindow * window_;

    // imgui member variables

};

}
