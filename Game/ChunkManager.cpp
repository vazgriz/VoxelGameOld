#include "ChunkManager.h"
#include "ChunkMesh.h"

ChunkManager::ChunkManager(entt::registry& chunkRegistry, FreeCam& freeCam) {
    m_registry = &chunkRegistry;
    m_freeCam = &freeCam;
}

void ChunkManager::update(VoxelEngine::Clock& clock) {
    glm::ivec3 worldChunk = Chunk::worldToWorldChunk(m_freeCam->position());
    worldChunk.y = 0;

    auto it = m_chunkMap.find(worldChunk);
    if (it == m_chunkMap.end()) {
        auto entity = m_registry->create();
        auto& chunk = m_registry->assign<Chunk>(entity, worldChunk);
        m_registry->assign<ChunkMesh>(entity);

        size_t i = 0;
        for (auto pos : Chunk::Positions()) {
            auto& block = chunk.blocks()[pos];
            block.type = (pos.x ^ pos.y ^ pos.z) & 1;
        }

        m_chunkMap[worldChunk] = &chunk;
    }
}