//
// Created by fangl on 2023/11/10.
//

#include "m1k_window.hpp"

// std
#include <stdexcept>

namespace m1k {

M1kWindow::M1kWindow(int w, int h, std::string name)
    : kWidth(w), kHeight(h), window_name_(name)
{
    initWindow();
}

M1kWindow::~M1kWindow() {
    glfwDestroyWindow(window_);
    glfwTerminate();
}

void M1kWindow::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window_ = glfwCreateWindow(kWidth, kHeight, window_name_.c_str(), nullptr, nullptr);
}

void M1kWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
    if(glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
}


}


