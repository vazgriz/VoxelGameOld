#include "Engine/Input.h"

using namespace VoxelEngine;

Input::Input(GLFWwindow* window)
    : m_onKeyChangedSignal(), m_onKeyChanged(m_onKeyChangedSignal)
    , m_onMouseButtonChangedSignal(), m_onMouseButtonChanged(m_onMouseButtonChangedSignal)
    , m_onMouseMovedSignal(), m_onMouseMoved(m_onMouseMovedSignal)
{
    m_window = window;
}

void Input::preUpdate() {
    m_delta = {};

    m_keyDowns.clear();
    m_keyUps.clear();

    m_mouseButtonDowns.clear();
    m_mouseButtonUps.clear();
}

void Input::handleKeyInput(int key_, int scancode, int action, int mods) {
    Key key = static_cast<Key>(key_);
    auto it = m_keyStates.find(key);

    if (it == m_keyStates.end()) {
        if (action == GLFW_PRESS) {
            auto pair = m_keyStates.insert(key);

            if (pair.second) {
                m_keyDowns.insert(key);
                m_onKeyChangedSignal.publish(key, KeyState::Down);
            }
        }
    } else {
        if (action == GLFW_RELEASE) {
            auto it = m_keyStates.find(key);

            if (it != m_keyStates.end()) {
                m_keyStates.erase(key);
                m_keyUps.insert(key);
                m_onKeyChangedSignal.publish(key, KeyState::Up);
            }
        }
    }
}

void Input::handleMouseButtonInput(int mouseButton, int action, int mods) {
    MouseButton button = static_cast<MouseButton>(mouseButton);
    auto it = m_mouseButtonStates.find(button);

    if (it == m_mouseButtonStates.end()) {
        if (action == GLFW_PRESS) {
            auto pair = m_mouseButtonStates.insert(button);

            if (pair.second) {
                m_mouseButtonDowns.insert(button);
                m_onMouseButtonChangedSignal.publish(button, KeyState::Down);
            }
        }
    } else {
        if (action == GLFW_RELEASE) {
            auto it = m_mouseButtonStates.find(button);

            if (it != m_mouseButtonStates.end()) {
                m_mouseButtonStates.erase(button);
                m_mouseButtonUps.insert(button);
                m_onMouseButtonChangedSignal.publish(button, KeyState::Up);
            }
        }
    }
}

void Input::handleMousePosition(double x, double y) {
    glm::vec2 newPosition = { static_cast<float>(x), static_cast<float>(y) };

    m_delta = newPosition - m_mousePosition;
    m_mousePosition = newPosition;
    m_onMouseMovedSignal.publish(m_delta);
}

KeyState Input::keyState(Key key) const {
    KeyState state = KeyState::None;

    if (keyDown(key)) {
        state |= KeyState::Down;
    }

    if (keyHold(key)) {
        state |= KeyState::Hold;
    }

    if (keyUp(key)) {
        state |= KeyState::Up;
    }

    return state;
}

bool Input::keyDown(Key key) const {
    return m_keyDowns.find(key) != m_keyDowns.end();
}

bool Input::keyHold(Key key) const {
    return m_keyStates.find(key) != m_keyStates.end();
}

bool Input::keyUp(Key key) const {
    return m_keyUps.find(key) != m_keyUps.end();
}

KeyState Input::mouseButtonState(MouseButton mouseButton) const {
    KeyState state = KeyState::None;

    if (mouseButtonDown(mouseButton)) {
        state |= KeyState::Down;
    }

    if (mouseButtonHold(mouseButton)) {
        state |= KeyState::Hold;
    }

    if (mouseButtonUp(mouseButton)) {
        state |= KeyState::Up;
    }

    return state;
}

bool Input::mouseButtonDown(MouseButton mouseButton) const {
    return m_mouseButtonDowns.find(mouseButton) != m_mouseButtonDowns.end();
}

bool Input::mouseButtonHold(MouseButton mouseButton) const {
    return m_mouseButtonStates.find(mouseButton) != m_mouseButtonStates.end();
}

bool Input::mouseButtonUp(MouseButton mouseButton) const {
    return m_mouseButtonUps.find(mouseButton) != m_mouseButtonUps.end();
}

void Input::setCursorState(CursorState state) {
    if (state == m_cursorState) return;

    if (state == CursorState::Normal) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else if (state == CursorState::Locked) {
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        return;
    }

    m_cursorState = state;

    double x, y;
    glfwGetCursorPos(m_window, &x, &y);
    m_mousePosition = { static_cast<float>(x), static_cast<float>(y) };
    m_delta = {};
}

glm::vec2 Input::mouseDelta() const {
    return m_delta;
}