#pragma once
#include <entt/entt.hpp>
#include <unordered_map>
#include <Engine/math.h>
#include <shared_mutex>
#include <glm/gtx/hash.hpp>
#include "Chunk.h"
#include "ChunkMesh.h"

class World;
class ChunkUpdater;

class ChunkGroup {
public:
    ChunkGroup(glm::ivec2 coord, entt::registry& registry);
    ChunkGroup(const ChunkGroup& other) = delete;
    ChunkGroup& operator = (const ChunkGroup& other) = delete;
    ChunkGroup(ChunkGroup&& other) = default;
    ChunkGroup& operator = (ChunkGroup&& other) = default;
    ~ChunkGroup();

    const std::vector<entt::entity>& chunks() const { return m_chunks; }

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

class World {
public:
    static const size_t worldHeight = 16;
    using ChunkMap = std::unordered_map<glm::ivec2, ChunkGroup>;

    World(int32_t viewDistance);

    void setChunkUpdater(ChunkUpdater& chunkUpdater);

    std::shared_lock<std::shared_mutex> getReadLock();
    entt::registry& registry() { return m_registry; }

    void update(glm::ivec2 coord);

private:
    std::shared_mutex m_mutex;
    entt::registry m_registry;
    ChunkMap m_chunkMap;
    int32_t m_viewDistance;
    int32_t m_viewDistance2;
    ChunkUpdater* m_chunkUpdater;

    std::queue<entt::entity> m_updateQueue;

    ChunkGroup& makeChunkGroup(glm::ivec2 coord);
    ChunkMap::iterator destroyChunkGroup(ChunkMap::iterator it, glm::ivec2 coord);
    static int32_t distance2(glm::ivec2 a, glm::ivec2 b);
};