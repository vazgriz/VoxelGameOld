#pragma once
#include <Engine/Engine.h>
#include <Engine/math.h>

class FreeCam : public VoxelEngine::System {
public:
    FreeCam(uint32_t priority, VoxelEngine::Camera& camera, VoxelEngine::Input& input);

    void update(VoxelEngine::Clock& clock);

private:
    VoxelEngine::Camera* m_camera;
    VoxelEngine::Input* m_input;

    bool m_locked = false;
    glm::vec2 m_look;
    glm::vec3 m_position;
    glm::quat m_rotation;
};