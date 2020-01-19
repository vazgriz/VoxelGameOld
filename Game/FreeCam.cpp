#include "FreeCam.h"
#include <algorithm>
#include <cmath>
#include <glm/ext/quaternion_transform.hpp>

using Key = VoxelEngine::Key;
using MouseButton = VoxelEngine::MouseButton;

FreeCam::FreeCam(uint32_t priority, VoxelEngine::Camera& camera, VoxelEngine::Input& input) : System(priority) {
    m_camera = &camera;
    m_input = &input;

    m_look = {};
    m_position = { 0, 0, 2 };
    m_rotation = glm::identity<glm::quat>();
}

void FreeCam::update(VoxelEngine::Clock& clock) {
    if (!m_locked && m_input->mouseButtonDown(MouseButton::Button1)) {
        m_locked = true;
        m_input->setCursorState(VoxelEngine::CursorState::Locked);
    }

    if (m_locked && m_input->keyDown(Key::Escape)) {
        m_locked = false;
        m_input->setCursorState(VoxelEngine::CursorState::Normal);
    }

    if (m_locked) {
        m_look += m_input->mouseDelta() * 0.05f;
        m_look.x = fmod(m_look.x, 360.0f);
        m_look.y = std::clamp<float>(m_look.y, -80, 80);

        m_rotation = glm::rotate(glm::identity<glm::quat>(), glm::radians(-m_look.x), glm::vec3{ 0, 1, 0 }) * glm::rotate(glm::identity<glm::quat>(), glm::radians(-m_look.y), glm::vec3{ 1, 0, 0 });

        glm::vec3 velocity = {};

        if (m_input->keyHold(Key::W)) {
            velocity += glm::vec3(0, 0, -1);
        }

        if (m_input->keyHold(Key::S)) {
            velocity += glm::vec3(0, 0, 1);
        }

        if (m_input->keyHold(Key::A)) {
            velocity += glm::vec3(-1, 0, 0);
        }

        if (m_input->keyHold(Key::D)) {
            velocity += glm::vec3(1, 0, 0);
        }

        if (m_input->keyHold(Key::E)) {
            velocity += glm::vec3(0, 1, 0);
        }

        if (m_input->keyHold(Key::Q)) {
            velocity += glm::vec3(0, -1, 0);
        }

        m_position += m_rotation * (velocity * clock.delta());
    }

    m_camera->setPosition(m_position);
    m_camera->setRotation(m_rotation);
}