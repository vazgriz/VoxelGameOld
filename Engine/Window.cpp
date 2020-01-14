#include <Engine/Window.h>

Window::Window(uint32_t width, uint32_t height, const std::string& title) {
    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
}

Window::~Window() {
    glfwDestroyWindow(m_window);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}