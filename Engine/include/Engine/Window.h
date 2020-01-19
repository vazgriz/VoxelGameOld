#pragma once
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <string>
#include <memory>

namespace VoxelEngine {
    class Input;

    class Window {
    public:
        Window(uint32_t width, uint32_t height, const std::string& title);
        ~Window();

        GLFWwindow* handle() const { return m_window; }
        Input& input() const { return *m_input; }

        uint32_t getWidth() const { return m_width; }
        uint32_t getHeight() const { return m_height; }

        bool shouldClose() const;
        void setTitle(const std::string& title);

    private:
        GLFWwindow* m_window;
        uint32_t m_width;
        uint32_t m_height;

        std::unique_ptr<Input> m_input;

        static void handleKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods);
    };
}