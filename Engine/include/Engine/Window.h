#pragma once
#include <GLFW/glfw3.h>
#include <stdint.h>
#include <string>
#include <memory>
#include <entt/signal/sigh.hpp>

namespace VoxelEngine {
    class Engine;
    class Input;

    class Window {
        friend class Engine;
    public:
        Window(uint32_t width, uint32_t height, const std::string& title);
        ~Window();

        GLFWwindow* handle() const { return m_window; }
        Input& input() const { return *m_input; }

        uint32_t getWidth() const { return m_width; }
        uint32_t getHeight() const { return m_height; }
        uint32_t getFramebufferWidth() const { return m_framebufferWidth; }
        uint32_t getFramebufferHeight() const { return m_framebufferHeight; }

        entt::sink<void(uint32_t, uint32_t)>& onResized() { return m_onResized; }
        entt::sink<void(uint32_t, uint32_t)>& onFramebufferResized() { return m_onFramebufferResized; }

        bool minimized() const;
        bool shouldClose() const;
        void setTitle(const std::string& title);

    private:
        GLFWwindow* m_window;
        uint32_t m_width;
        uint32_t m_height;
        uint32_t m_framebufferWidth;
        uint32_t m_framebufferHeight;

        std::unique_ptr<Input> m_input;

        bool m_resized = false;
        bool m_minimized = false;

        entt::sigh<void(uint32_t, uint32_t)> m_onResizedSignal;
        entt::sink<void(uint32_t, uint32_t)> m_onResized;

        entt::sigh<void(uint32_t, uint32_t)> m_onFramebufferResizedSignal;
        entt::sink<void(uint32_t, uint32_t)> m_onFramebufferResized;

        void update();

        static void handleWindowResize(GLFWwindow* window, int width, int height);
        static void handleFramebufferResize(GLFWwindow* window, int width, int height);
        static void handleIconify(GLFWwindow* window, int iconified);

        static void handleKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void handleMouseButtonInput(GLFWwindow* window, int mouseButton, int action, int mods);
        static void handleMousePosition(GLFWwindow* window, double xpos, double ypos);
    };
}