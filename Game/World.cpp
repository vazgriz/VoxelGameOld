#include "World.h"
#include "Chunk.h"
#include "ChunkMesh.h"

World::World() {

}

std::shared_lock<std::shared_mutex> World::getReadLock() {
    return std::shared_lock<std::shared_mutex>(m_mutex);
}

std::unique_lock<std::shared_mutex> World::getWriteLock() {
    return std::unique_lock<std::shared_mutex>(m_mutex);
}

entt::entity World::createChunk(glm::ivec3 worldChunkPos) {
    entt::entity chunkEntity = m_registry.create();
    m_registry.assign<Chunk>(chunkEntity, worldChunkPos);
    m_registry.assign<ChunkMesh>(chunkEntity);

    m_chunkMap.insert({ worldChunkPos, chunkEntity });

    return chunkEntity;
}

void World::destroyChunk(glm::ivec3 worldChunkPos, entt::entity entity) {
    if (m_chunkMap.erase(worldChunkPos) == 1) {
        m_registry.destroy(entity);
    }
}

entt::entity World::getEntity(glm::ivec3 worldChunkPos) {
    auto it = m_chunkMap.find(worldChunkPos);
    if (it != m_chunkMap.end()) {
        return it->second;
    } else {
        return entt::null;
    }
}

Chunk* World::getChunk(glm::ivec3 worldChunkPos) {
    auto entity = getEntity(worldChunkPos);
    if (entity == entt::null) {
        return nullptr;
    }

    return &m_registry.view<Chunk>().get(entity);
}

ChunkMesh* World::getChunkMesh(glm::ivec3 worldChunkPos) {
    auto entity = getEntity(worldChunkPos);
    if (entity == entt::null) {
        return nullptr;
    }

    return &m_registry.view<ChunkMesh>().get(entity);
}