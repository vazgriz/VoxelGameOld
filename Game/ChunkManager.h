#pragma once
#include <entt/entt.hpp>
#include <Engine/System.h>
#include <unordered_map>
#include "FreeCam.h"
#include "Chunk.h"
#include "World.h"

class ChunkManager : public VoxelEngine::System {
public:
    static const int32_t worldHeight = 16;

    ChunkManager(World& world, FreeCam& freeCam);

    void update(VoxelEngine::Clock& clock);

private:
    World* m_world;
    FreeCam* m_freeCam;
    glm::ivec3 m_lastPos;
};