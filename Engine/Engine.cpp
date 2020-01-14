#include <Engine/Engine.h>
#include <GLFW/glfw3.h>
#include <iostream>

Engine::Engine() {
    glfwSetErrorCallback([](int errorCode, const char* description) {
        std::cout << "GLFW error: " << description << std::endl;
    });
    glfwInit();
}

Engine::~Engine() {
    glfwTerminate();
}

void Engine::addWindow(Window& window) {
    m_window = &window;
}

void Engine::run() {
    while (!m_window->shouldClose()) {
        glfwPollEvents();
    }
}