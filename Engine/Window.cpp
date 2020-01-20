#include "Engine/Window.h"
#include "Engine/Input.h"

using namespace VoxelEngine;

Window::Window(uint32_t width, uint32_t height, const std::string& title)
    : m_onResizedSignal(), m_onResized(m_onResizedSignal)
    , m_onFramebufferResizedSignal(), m_onFramebufferResized(m_onFramebufferResizedSignal)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

    m_input = std::unique_ptr<Input>(new Input(m_window));

    glfwSetWindowUserPointer(m_window, this);

    glfwSetWindowSizeCallback(m_window, &handleWindowResize);
    glfwSetFramebufferSizeCallback(m_window, &handleFramebufferResize);

    glfwSetKeyCallback(m_window, &handleKeyInput);
    glfwSetMouseButtonCallback(m_window, &handleMouseButtonInput);
    glfwSetCursorPosCallback(m_window, &handleMousePosition);

    int windowWidth, windowHeight;
    glfwGetWindowSize(m_window, &windowWidth, &windowHeight);
    m_width = static_cast<uint32_t>(windowWidth);
    m_height = static_cast<uint32_t>(windowHeight);

    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);
    m_framebufferWidth = static_cast<uint32_t>(framebufferWidth);
    m_framebufferHeight = static_cast<uint32_t>(framebufferHeight);
}

Window::~Window() {
    glfwDestroyWindow(m_window);
}

void Window::update() {
    m_input->preUpdate();
    glfwPollEvents();

    if (m_resized) {
        m_resized = false;
        m_onResizedSignal.publish(m_width, m_height);
        m_onFramebufferResizedSignal.publish(m_framebufferWidth, m_framebufferHeight);
    }
}

void Window::handleWindowResize(GLFWwindow* window_, int width, int height) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(window_));
    window->m_resized = true;
    window->m_width = static_cast<uint32_t>(width);
    window->m_height = static_cast<uint32_t>(height);
}

void Window::handleFramebufferResize(GLFWwindow* window_, int width, int height) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(window_));
    window->m_resized = true;
    window->m_framebufferWidth = static_cast<uint32_t>(width);
    window->m_framebufferHeight = static_cast<uint32_t>(height);
}

void Window::handleKeyInput(GLFWwindow* window_, int key, int scancode, int action, int mods) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(window_));
    Input* input = window->m_input.get();
    input->handleKeyInput(key, scancode, action, mods);
}

void Window::handleMouseButtonInput(GLFWwindow* window_, int mouseButton, int action, int mods) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(window_));
    Input* input = window->m_input.get();
    input->handleMouseButtonInput(mouseButton, action, mods);
}

void Window::handleMousePosition(GLFWwindow* window_, double xpos, double ypos) {
    Window* window = static_cast<Window*>(glfwGetWindowUserPointer(window_));
    Input* input = window->m_input.get();
    input->handleMousePosition(xpos, ypos);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void Window::setTitle(const std::string& title) {
    glfwSetWindowTitle(m_window, title.c_str());
}