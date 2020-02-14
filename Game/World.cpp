#include "World.h"
#include "ChunkMesh.h"

Block World::m_nullBlock = Block();
Block World::m_airBlock = Block(1);

World::World() {

}

std::shared_lock<std::shared_mutex> World::getReadLock() {
    return std::shared_lock<std::shared_mutex>(m_mutex);
}

std::unique_lock<std::shared_mutex> World::getWriteLock() {
    return std::unique_lock<std::shared_mutex>(m_mutex);
}

entt::entity World::createChunk(glm::ivec3 worldChunkPos) {
    entt::entity chunkEntity;
    
    if (m_recycleQueue.size() > 0) {
        chunkEntity = m_recycleQueue.front();
        m_recycleQueue.pop();

        auto view = m_registry.view<Chunk>();
        view.get(chunkEntity).setWorldChunkPosition(worldChunkPos);
    } else {
        chunkEntity = m_registry.create();
        m_registry.assign<Chunk>(chunkEntity, chunkEntity, worldChunkPos);
        m_registry.assign<ChunkMesh>(chunkEntity);
    }

    m_chunkMap.insert({ worldChunkPos, chunkEntity });
    m_chunkSet.insert(chunkEntity);

    return chunkEntity;
}

void World::destroyChunk(glm::ivec3 worldChunkPos, entt::entity entity) {
    if (m_chunkMap.erase(worldChunkPos) == 1) {
        m_chunkSet.erase(entity);
        m_recycleQueue.push(entity);
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

bool World::valid(glm::ivec3 coord) {
    return getEntity(coord) != entt::null;
}

bool World::valid(entt::entity entity) {
    return m_chunkSet.count(entity) != 0;
}

Block& World::getBlock(glm::ivec3 worldPos) {
    if (worldPos.y >= worldHeight * Chunk::chunkSize || worldPos.y < 0) {
        return m_airBlock;
    }

    glm::ivec3 worldChunkPos = Chunk::worldToWorldChunk(worldPos);
    Chunk* chunk = getChunk(worldChunkPos);

    if (chunk != nullptr) {
        return chunk->blocks()[Chunk::worldToChunk(worldPos)];
    } else {
        return m_nullBlock;
    }
}

void World::update(glm::ivec3 worldPosition) {
    auto worldChunkPos = Chunk::worldToWorldChunk(worldPosition);

    auto it = m_chunkMap.find(worldChunkPos);

    if (it != m_chunkMap.end()) {
        auto chunkEntity = it->second;
        update(worldChunkPos, chunkEntity);
    }
}

void World::update(glm::ivec3 worldChunkPos, entt::entity entity) {
    m_worldUpdates.enqueue({ worldChunkPos, entity });
}