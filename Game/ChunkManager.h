#pragma once
#include <entt/entt.hpp>
#include <Engine/System.h>
#include <unordered_map>
#include "FreeCam.h"
#include "Chunk.h"
#include "glm/gtx/hash.hpp"

class ChunkGroup {
public:
    ChunkGroup(glm::ivec2 coord, entt::registry& registry);
    ~ChunkGroup();

    ChunkLoadState loadState() const { return m_loadState; }
    void setLoadState(ChunkLoadState loadState);

    ChunkGroup* getNeighbor(ChunkDirection dir);
    ChunkGroup* getNeighbor(glm::ivec2 offset);
    ChunkGroup* getNeighbor(glm::ivec3 offset);
    void setNeighbor(ChunkDirection dir, ChunkGroup* group);

    static ChunkDirection getOpposite(ChunkDirection dir);
    static ChunkDirection getDirection(glm::ivec2 offset);

private:
    ChunkLoadState m_loadState;
    glm::ivec2 m_coord;
    entt::registry* m_registry;
    std::vector<entt::entity> m_chunks;

    std::array<ChunkGroup*, 8> m_neighbors;
};

class ChunkManager : public VoxelEngine::System {
public:
    static const int32_t worldHeight = 16;

    ChunkManager(entt::registry& chunkRegistry, FreeCam& freeCam, int32_t viewDistance);

    void update(VoxelEngine::Clock& clock);

private:
    using ChunkMap = std::unordered_map<glm::ivec2, std::unique_ptr<ChunkGroup>>;
    entt::registry* m_registry;
    FreeCam* m_freeCam;
    ChunkMap m_chunkMap;
    glm::ivec3 m_lastPos;
    int32_t m_viewDistance;
    int32_t m_viewDistance2;

    ChunkGroup& makeChunkGroup(glm::ivec2 coord);
    ChunkMap::iterator destroyChunkGroup(ChunkMap::iterator it, glm::ivec2 coord);

    static int32_t distance2(glm::ivec2 a, glm::ivec2 b);
};