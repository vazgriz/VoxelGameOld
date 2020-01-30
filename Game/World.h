#pragma once
#include <entt/entt.hpp>
#include <unordered_map>
#include <Engine/math.h>
#include <shared_mutex>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class Chunk;
class ChunkMesh;

class World {
public:
    static const size_t worldHeight = 16;

    World();

    std::shared_lock<std::shared_mutex> getReadLock();
    std::unique_lock<std::shared_mutex> getWriteLock();

    entt::entity createChunk(glm::ivec3 worldChunkPos);
    void destroyChunk(glm::ivec3 worldChunkPos, entt::entity entity);

    entt::registry& registry() { return m_registry; }
    entt::entity getEntity(glm::ivec3 worldChunkPos);
    Chunk* getChunk(glm::ivec3 worldChunkPos);
    ChunkMesh* getChunkMesh(glm::ivec3 worldChunkPos);

private:
    std::shared_mutex m_mutex;
    entt::registry m_registry;
    std::unordered_map<glm::ivec3, entt::entity> m_chunkMap;
};