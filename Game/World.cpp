#include "World.h"
#include "ChunkMesh.h"
#include "BlockManager.h"

Block World::m_nullBlock = Block();
Block World::m_airBlock = Block(1);

World::World(BlockManager& blockManager) {
    m_blockManager = &blockManager;
}

std::unique_lock<std::mutex> World::getLock() {
    return std::unique_lock<std::mutex>(m_mutex);
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
        m_registry.assign<Chunk>(chunkEntity, chunkEntity, worldChunkPos, *this);
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

void World::queueChunkUpdate(glm::ivec3 worldChunkPos) {
    m_worldUpdates.enqueue(worldChunkPos);
}

std::optional<RaycastResult> World::raycast(glm::vec3 origin, glm::vec3 dir, float distance) {
    auto lock = getLock();

    float t = 0.0f;

    glm::ivec3 i = glm::ivec3(
        static_cast<int32_t>(floor(origin.x)),
        static_cast<int32_t>(floor(origin.y)),
        static_cast<int32_t>(floor(origin.z))
    );

    auto worldPos = Chunk::split(i);

    glm::ivec3 chunkPos = worldPos[0];
    glm::ivec3 pos = worldPos[1];

    glm::ivec3 step = glm::ivec3(
        (dir.x > 0) ? 1 : -1,
        (dir.y > 0) ? 1 : -1,
        (dir.z > 0) ? 1 : -1
    );

    glm::vec3 tDelta = glm::vec3(abs(1 / dir.x), abs(1 / dir.y), abs(1 / dir.z));
    glm::vec3 dist = glm::vec3(
        (step.x > 0) ? (i.x + 1 - origin.x) : (origin.x - i.x),
        (step.y > 0) ? (i.y + 1 - origin.y) : (origin.y - i.y),
        (step.z > 0) ? (i.z + 1 - origin.z) : (origin.z - i.z)
    );

    glm::vec3 max = tDelta * dist;
    int32_t steppedIndex = -1;

    auto view = m_registry.view<Chunk>();
    Chunk* currentChunk = getChunk(chunkPos);

    while (t <= distance) {
        if (!Chunk::chunkPosInBounds(pos)) {
            auto worldPos = Chunk::split(i);

            chunkPos = worldPos[0];
            pos = worldPos[1];

            currentChunk = getChunk(chunkPos);
        }

        if (currentChunk != nullptr) {
            Block& block = currentChunk->blocks()[pos];
            BlockType& type = m_blockManager->getType(block);

            if (type.solid()) {
                RaycastResult result;
                result.blockPosition = i;
                result.position = origin + t * dir;
                result.normal = {};
                result.normal[steppedIndex] = -step[steppedIndex];
                result.chunk = currentChunk;

                return result;
            }
        }

        if (max.x < max.y) {
            if (max.x < max.z) {
                i.x += step.x;
                pos.x += step.x;
                t = max.x;
                max.x += tDelta.x;
                steppedIndex = 0;
            } else {
                i.z += step.z;
                pos.z += step.z;
                t = max.z;
                max.z += tDelta.z;
                steppedIndex = 2;
            }
        } else {
            if (max.y < max.z) {
                i.y += step.y;
                pos.y += step.y;
                t = max.y;
                max.y += tDelta.y;
                steppedIndex = 1;
            } else {
                i.z += step.z;
                pos.z += step.z;
                t = max.z;
                max.z += tDelta.z;
                steppedIndex = 2;
            }
        }
    }

    return {};
}