#pragma once
#include <entt/entt.hpp>
#include <Engine/System.h>
#include <unordered_map>
#include "FreeCam.h"
#include "Chunk.h"
#include "glm/gtx/hash.hpp"

class ChunkManager : public VoxelEngine::System {
public:
    ChunkManager(entt::registry& chunkRegistry, FreeCam& freeCam);

    void update(VoxelEngine::Clock& clock);

private:
    entt::registry* m_registry;
    FreeCam* m_freeCam;
    std::unordered_map<glm::ivec3, Chunk*> m_chunkMap;
    glm::ivec3 m_lastPos;
};