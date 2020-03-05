#pragma once
#include <entt/entt.hpp>
#include <unordered_map>
#include <unordered_set>
#include <Engine/math.h>
#include <Engine/BufferedQueue.h>
#include <shared_mutex>
#include <optional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "Chunk.h"

class ChunkMesh;
class BlockManager;

struct RaycastResult {
    glm::vec3 position;
    glm::ivec3 blockPosition;
    glm::vec3 normal;
    Chunk* chunk;
};

class World {
public:
    static const size_t worldHeight = 16;

    World(BlockManager& blockManager);

    std::shared_lock<std::shared_mutex> getReadLock();
    std::unique_lock<std::shared_mutex> getWriteLock();

    entt::entity createChunk(glm::ivec3 worldChunkPos);
    void destroyChunk(glm::ivec3 worldChunkPos, entt::entity entity);

    entt::registry& registry() { return m_registry; }
    entt::entity getEntity(glm::ivec3 worldChunkPos);
    Chunk* getChunk(glm::ivec3 worldChunkPos);
    ChunkMesh* getChunkMesh(glm::ivec3 worldChunkPos);

    bool valid(glm::ivec3 coord);
    bool valid(entt::entity entity);

    Block& getBlock(glm::ivec3 worldPos);

    void update(glm::ivec3 worldChunkPos);

    std::queue<glm::ivec3>& getUpdates() { return m_worldUpdates.swapDequeue(); }

    std::optional<RaycastResult> raycast(glm::vec3 origin, glm::vec3 dir, float distance);

private:
    static Block m_nullBlock;
    static Block m_airBlock;
    BlockManager* m_blockManager;
    std::shared_mutex m_mutex;
    entt::registry m_registry;
    std::unordered_set<entt::entity> m_chunkSet;
    std::unordered_map<glm::ivec3, entt::entity> m_chunkMap;
    VoxelEngine::BufferedQueue<glm::ivec3> m_worldUpdates;
    std::queue<entt::entity> m_recycleQueue;
};