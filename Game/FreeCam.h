#pragma once
#include <Engine/Engine.h>
#include <Engine/math.h>
#include <optional>
#include "World.h"
#include "SelectionBox.h"

class World;
class BlockManager;

class FreeCam : public VoxelEngine::System {
public:
    FreeCam(VoxelEngine::Camera& camera, VoxelEngine::Input& input, World& world, BlockManager& blockManager, SelectionBox& selectionBox);

    glm::vec3 position() { return m_position; }
    void setPosition(glm::vec3 pos);

    void update(VoxelEngine::Clock& clock);

private:
    VoxelEngine::Camera* m_camera;
    VoxelEngine::Input* m_input;
    World* m_world;
    BlockManager* m_blockManager;
    SelectionBox* m_selectionBox;

    bool m_locked = false;
    glm::vec2 m_look;
    glm::vec3 m_position;
    glm::quat m_rotation;
    int32_t m_placeType;

    void handleKeyInput(VoxelEngine::Key key, VoxelEngine::KeyState);
};