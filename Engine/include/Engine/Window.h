#pragma once
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <string>

class Window {
public:
    Window(uint32_t width, uint32_t height, const std::string& title);
    ~Window();

    bool shouldClose() const;
    void setTitle(const std::string& title);

private:
    GLFWwindow* m_window;
};