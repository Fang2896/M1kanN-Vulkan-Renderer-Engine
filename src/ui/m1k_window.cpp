//
// Created by fangl on 2023/11/10.
//

#include "m1k_window.hpp"

// std
#include <stdexcept>

namespace m1k {

M1kWindow::M1kWindow(int w, int h, std::string name)
    : width_(w), height_(h), window_name_(name)
{
    initWindow();
}

M1kWindow::~M1kWindow() {
    glfwDestroyWindow(window_);
    glfwTerminate();
}

void M1kWindow::framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    auto m1k_window = reinterpret_cast<M1kWindow*>(glfwGetWindowUserPointer(window));
    m1k_window->framebuffer_resized_ = true;
    m1k_window->width_ = width;
    m1k_window->height_ = height;
}

void M1kWindow::initWindow() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window_ = glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);

    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebufferResizeCallback);
}

void M1kWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if(glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window_ surface");
    }
}


}


