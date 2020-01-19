#include "Engine/Input.h"

using namespace VoxelEngine;

Input::Input(GLFWwindow* window) : m_onKeyChangedSignal(), m_onKeyChanged(m_onKeyChangedSignal) {
    m_window = window;
}

void Input::handleInput(int key_, int scancode, int action, int mods) {
    m_keyDowns.clear();
    m_keyUps.clear();

    Input::Key key = static_cast<Input::Key>(key_);
    auto it = m_keyStates.find(key);

    if (it == m_keyStates.end()) {
        if (action == GLFW_PRESS) {
            m_keyStates.insert(key);
            m_keyDowns.insert(key);

            m_onKeyChangedSignal.publish(key, KeyState::Down);
        }
    } else {
        if (action == GLFW_RELEASE) {
            m_keyStates.erase(key);
            m_keyUps.insert(key);

            m_onKeyChangedSignal.publish(key, KeyState::Up);
        }
    }
}

Input::KeyState Input::keyState(Key key) const {
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