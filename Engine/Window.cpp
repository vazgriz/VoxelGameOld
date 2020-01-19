#include "Engine/Window.h"
#include "Engine/Input.h"

using namespace VoxelEngine;

Window::Window(uint32_t width, uint32_t height, const std::string& title) {
    m_width = width;
    m_height = height;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    m_input = std::unique_ptr<Input>(new Input(m_window));

    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, &handleKeyInput);
}

Window::~Window() {
    glfwDestroyWindow(m_window);
}

void Window::handleKeyInput(GLFWwindow* window_, int key, int scancode, int action, int mods) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(window_));
    Input* input = window->m_input.get();
    input->handleInput(key, scancode, action, mods);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::setTitle(const std::string& title) {
    glfwSetWindowTitle(m_window, title.c_str());
}