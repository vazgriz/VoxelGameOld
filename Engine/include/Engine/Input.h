#pragma once
#include "Engine/Engine.h"
#include "Engine/math.h"
#include "GLFW/glfw3.h"
#include "entt/signal/sigh.hpp"
#include <unordered_set>

namespace VoxelEngine {
    class Input {
        friend class Window;
    public:
        enum class Key : int32_t {
            Unknown = -1,
            None = 0,
            Space = GLFW_KEY_SPACE,
            Apostrophe = GLFW_KEY_APOSTROPHE,
            Comma = GLFW_KEY_COMMA,
            Minus = GLFW_KEY_MINUS,
            Period = GLFW_KEY_PERIOD,
            Slash = GLFW_KEY_SLASH,
            Key0 = GLFW_KEY_0,
            Key1 = GLFW_KEY_1,
            Key2 = GLFW_KEY_2,
            Key3 = GLFW_KEY_3,
            Key4 = GLFW_KEY_4,
            Key5 = GLFW_KEY_5,
            Key6 = GLFW_KEY_6,
            Key7 = GLFW_KEY_7,
            Key8 = GLFW_KEY_8,
            Key9 = GLFW_KEY_9,
            Semicolon = GLFW_KEY_SEMICOLON,
            Equal = GLFW_KEY_EQUAL,
            A = GLFW_KEY_A,
            B = GLFW_KEY_B,
            C = GLFW_KEY_C,
            D = GLFW_KEY_D,
            E = GLFW_KEY_E,
            F = GLFW_KEY_F,
            G = GLFW_KEY_G,
            H = GLFW_KEY_H,
            I = GLFW_KEY_I,
            J = GLFW_KEY_J,
            K = GLFW_KEY_K,
            L = GLFW_KEY_L,
            M = GLFW_KEY_M,
            N = GLFW_KEY_N,
            O = GLFW_KEY_O,
            P = GLFW_KEY_P,
            Q = GLFW_KEY_Q,
            R = GLFW_KEY_R,
            S = GLFW_KEY_S,
            T = GLFW_KEY_T,
            U = GLFW_KEY_U,
            V = GLFW_KEY_V,
            W = GLFW_KEY_W,
            X = GLFW_KEY_X,
            Y = GLFW_KEY_Y,
            Z = GLFW_KEY_Z,
            LeftBracket = GLFW_KEY_LEFT_BRACKET,
            Backslash = GLFW_KEY_BACKSLASH,
            RightBracket = GLFW_KEY_RIGHT_BRACKET,
            Grave = GLFW_KEY_GRAVE_ACCENT,
            World1 = GLFW_KEY_WORLD_1,
            World2 = GLFW_KEY_WORLD_2,
            Escape = GLFW_KEY_ESCAPE,
            Enter = GLFW_KEY_ENTER,
            Tab = GLFW_KEY_TAB,
            Backspace = GLFW_KEY_BACKSPACE,
            Insert = GLFW_KEY_INSERT,
            Delete = GLFW_KEY_DELETE,
            Right = GLFW_KEY_RIGHT,
            Left = GLFW_KEY_LEFT,
            Down = GLFW_KEY_DOWN,
            Up = GLFW_KEY_UP,
            PageUp = GLFW_KEY_PAGE_UP,
            PageDown = GLFW_KEY_PAGE_DOWN,
            Home = GLFW_KEY_HOME,
            End = GLFW_KEY_END,
            CapsLock = GLFW_KEY_CAPS_LOCK,
            ScrollLock = GLFW_KEY_SCROLL_LOCK,
            NumLock = GLFW_KEY_NUM_LOCK,
            PrintScreen = GLFW_KEY_PRINT_SCREEN,
            Pause = GLFW_KEY_PAUSE,
            F1 = GLFW_KEY_F1,
            F2 = GLFW_KEY_F2,
            F3 = GLFW_KEY_F3,
            F4 = GLFW_KEY_F4,
            F5 = GLFW_KEY_F5,
            F6 = GLFW_KEY_F6,
            F7 = GLFW_KEY_F7,
            F8 = GLFW_KEY_F8,
            F9 = GLFW_KEY_F9,
            F10 = GLFW_KEY_F10,
            F11 = GLFW_KEY_F11,
            F12 = GLFW_KEY_F12,
            F13 = GLFW_KEY_F13,
            F14 = GLFW_KEY_F14,
            F15 = GLFW_KEY_F15,
            F16 = GLFW_KEY_F16,
            F17 = GLFW_KEY_F17,
            F18 = GLFW_KEY_F18,
            F19 = GLFW_KEY_F19,
            F20 = GLFW_KEY_F20,
            F21 = GLFW_KEY_F21,
            F22 = GLFW_KEY_F22,
            F23 = GLFW_KEY_F23,
            F24 = GLFW_KEY_F24,
            F25 = GLFW_KEY_F25,
            Keypad0 = GLFW_KEY_KP_0,
            Keypad1 = GLFW_KEY_KP_1,
            Keypad2 = GLFW_KEY_KP_2,
            Keypad3 = GLFW_KEY_KP_3,
            Keypad4 = GLFW_KEY_KP_4,
            Keypad5 = GLFW_KEY_KP_5,
            Keypad6 = GLFW_KEY_KP_6,
            Keypad7 = GLFW_KEY_KP_7,
            Keypad8 = GLFW_KEY_KP_8,
            Keypad9 = GLFW_KEY_KP_9,
            KeyPadDecimal = GLFW_KEY_KP_DECIMAL,
            KeyPadDivide = GLFW_KEY_KP_DIVIDE,
            KeyPadMultiply = GLFW_KEY_KP_MULTIPLY,
            KeyPadSubtract = GLFW_KEY_KP_SUBTRACT,
            KeyPadAdd = GLFW_KEY_KP_ADD,
            KeyPadEnter = GLFW_KEY_KP_ENTER,
            KeyPadEqual = GLFW_KEY_KP_EQUAL,
            LeftShift = GLFW_KEY_LEFT_SHIFT,
            LeftConstrol = GLFW_KEY_LEFT_CONTROL,
            LeftAlt = GLFW_KEY_LEFT_ALT,
            LeftSuper = GLFW_KEY_LEFT_SUPER,
            RightShift = GLFW_KEY_RIGHT_SHIFT,
            RightControl = GLFW_KEY_RIGHT_CONTROL,
            RightAlt = GLFW_KEY_RIGHT_ALT,
            RightSuper = GLFW_KEY_RIGHT_SUPER,
            Menu = GLFW_KEY_MENU,
        };

        enum class MouseButton : int32_t {
            Button1 = GLFW_MOUSE_BUTTON_1,
            Button2 = GLFW_MOUSE_BUTTON_2,
            Button3 = GLFW_MOUSE_BUTTON_3,
            Button4 = GLFW_MOUSE_BUTTON_4,
            Button5 = GLFW_MOUSE_BUTTON_5,
            Button6 = GLFW_MOUSE_BUTTON_6,
            Button7 = GLFW_MOUSE_BUTTON_7,
            Button8 = GLFW_MOUSE_BUTTON_8,
            Left = Button1,
            Right = Button2,
            Middle = Button3
        };

        enum class KeyState {
            None = 0,
            Down = 1,
            Hold = 2,
            Up = 4
        };

        enum class CursorState {
            Normal,
            Locked
        };

        KeyState keyState(Key key) const;
        bool keyDown(Key key) const;
        bool keyHold(Key key) const;
        bool keyUp(Key key) const;

        KeyState mouseButtonState(MouseButton mouseButton) const;
        bool mouseButtonDown(MouseButton mouseButton) const;
        bool mouseButtonHold(MouseButton mouseButton) const;
        bool mouseButtonUp(MouseButton mouseButton) const;

        void setCursorState(CursorState state);
        glm::vec2 mouseDelta() const;

        entt::sink<void(Key, KeyState)>& onKeyChanged() { return m_onKeyChanged;  }
        entt::sink<void(MouseButton, KeyState)>& onMouseButtonChanged() { return m_onMouseButtonChanged; }
        entt::sink<void(glm::vec2)>& onMouseMoved() { return m_onMouseMoved; }

    private:
        Input(GLFWwindow* window);

        GLFWwindow* m_window;

        entt::sigh<void(Key, KeyState)> m_onKeyChangedSignal;
        entt::sink<void(Key, KeyState)> m_onKeyChanged;
        entt::sigh<void(MouseButton, KeyState)> m_onMouseButtonChangedSignal;
        entt::sink<void(MouseButton, KeyState)> m_onMouseButtonChanged;
        entt::sigh<void(glm::vec2)> m_onMouseMovedSignal;
        entt::sink<void(glm::vec2)> m_onMouseMoved;

        std::unordered_set<Key> m_keyStates;
        std::unordered_set<Key> m_keyDowns;
        std::unordered_set<Key> m_keyUps;

        std::unordered_set<MouseButton> m_mouseButtonStates;
        std::unordered_set<MouseButton> m_mouseButtonDowns;
        std::unordered_set<MouseButton> m_mouseButtonUps;

        CursorState m_cursorState;
        glm::vec2 m_mousePosition;
        glm::vec2 m_delta;

        void preUpdate();

        void handleKeyInput(int key, int scancode, int action, int mods);
        void handleMouseButtonInput(int mouseButton, int action, int mods);
        void handleMousePosition(double x, double y);
    };

    static inline Input::KeyState operator | (Input::KeyState a, Input::KeyState b) {
        return static_cast<Input::KeyState>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    static inline Input::KeyState& operator |= (Input::KeyState& a, Input::KeyState b) {
        a = a | b;
        return a;
    }

    static inline Input::KeyState operator & (Input::KeyState a, Input::KeyState b) {
        return static_cast<Input::KeyState>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }

    static inline Input::KeyState& operator &= (Input::KeyState& a, Input::KeyState b) {
        a = a & b;
        return a;
    }
}