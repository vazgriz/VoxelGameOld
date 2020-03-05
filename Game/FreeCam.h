#pragma once
#include <Engine/Engine.h>
#include <Engine/math.h>
#include <optional>
#include "World.h"
#include "SelectionBox.h"

class World;

class FreeCam : public VoxelEngine::System {
public:
    FreeCam(VoxelEngine::Camera& camera, VoxelEngine::Input& input, World& world, SelectionBox& selectionBox);

    glm::vec3 position() { return m_position; }
    void setPosition(glm::vec3 pos);
    std::optional<RaycastResult>& raycastResult() { return m_raycastResult; }

    void update(VoxelEngine::Clock& clock);

private:
    VoxelEngine::Camera* m_camera;
    VoxelEngine::Input* m_input;
    World* m_world;
    SelectionBox* m_selectionBox;

    bool m_locked = false;
    glm::vec2 m_look;
    glm::vec3 m_position;
    glm::quat m_rotation;
    std::optional<RaycastResult> m_raycastResult;
};