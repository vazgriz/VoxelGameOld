#include "ChunkManager.h"
#include "ChunkMesh.h"
#include <iostream>

ChunkManager::ChunkManager(World& world, FreeCam& freeCam) {
    m_world = &world;
    m_freeCam = &freeCam;
}

void ChunkManager::update(VoxelEngine::Clock& clock) {
    glm::ivec3 worldChunk = Chunk::worldToWorldChunk(m_freeCam->position());
    worldChunk.y = 0;
    glm::ivec2 coord = glm::ivec2(worldChunk.x, worldChunk.z);

    m_world->update(coord);
}