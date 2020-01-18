#include <Engine/Window.h>

using namespace VoxelEngine;

Window::Window(uint32_t width, uint32_t height, const std::string& title) {
    m_width = width;
    m_height = height;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

Window::~Window() {
    glfwDestroyWindow(m_window);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::setTitle(const std::string& title) {
    glfwSetWindowTitle(m_window, title.c_str());
}