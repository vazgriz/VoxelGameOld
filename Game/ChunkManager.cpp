#include "ChunkManager.h"
#include "ChunkMesh.h"
#include <iostream>

ChunkGroup::ChunkGroup(glm::ivec2 coord, entt::registry& registry) : m_neighbors() {
    m_coord = coord;
    m_registry = &registry;
    m_loadState = ChunkLoadState::Loading;

    for (int32_t i = 0; i < ChunkManager::worldHeight; i++) {
        entt::entity chunkEntity = m_registry->create();
        auto& chunk = m_registry->assign<Chunk>(chunkEntity, glm::ivec3(coord.x, i, coord.y));
        m_registry->assign<ChunkMesh>(chunkEntity);

        for (auto pos : Chunk::Positions()) {
            auto& block = chunk.blocks()[pos];
            block.type = 3;
        }

        chunk.setLoadState(ChunkLoadState::Loaded);

        m_chunks.push_back(chunkEntity);
    }
}

ChunkGroup::~ChunkGroup() {
    for (auto entity : m_chunks) {
        m_registry->destroy(entity);
    }
}

void ChunkGroup::setLoadState(ChunkLoadState loadState) {
    m_loadState = loadState;

    auto view = m_registry->view<Chunk>();
    for (auto entity : m_chunks) {
        auto& chunk = view.get<Chunk>(entity);
        chunk.setLoadState(loadState);
    }
}

ChunkGroup* ChunkGroup::getNeighbor(ChunkDirection dir) {
    return m_neighbors[static_cast<size_t>(dir)];
}

ChunkGroup* ChunkGroup::getNeighbor(glm::ivec2 offset) {
    return m_neighbors[static_cast<size_t>(getDirection(offset))];
}

ChunkGroup* ChunkGroup::getNeighbor(glm::ivec3 offset) {
    return m_neighbors[static_cast<size_t>(getDirection(glm::ivec2(offset.x, offset.z)))];
}

void ChunkGroup::setNeighbor(ChunkDirection dir, ChunkGroup* group) {
    m_neighbors[static_cast<size_t>(dir)] = group;
}

ChunkDirection ChunkGroup::getOpposite(ChunkDirection dir) {
    return static_cast<ChunkDirection>((static_cast<size_t>(dir) + 2) & 7);
}

ChunkDirection ChunkGroup::getDirection(glm::ivec2 offset) {
    if (offset == glm::ivec2(0, -1)) {
        return ChunkDirection::North;
    } else if (offset == glm::ivec2(1, -1)) {
        return ChunkDirection::NorthEast;
    } else if (offset == glm::ivec2(1, 0)) {
        return ChunkDirection::East;
    } else if (offset == glm::ivec2(1, 1)) {
        return ChunkDirection::SouthEast;
    } else if (offset == glm::ivec2(0, 1)) {
        return ChunkDirection::South;
    } else if (offset == glm::ivec2(-1, 1)) {
        return ChunkDirection::SouthWest;
    } else if (offset == glm::ivec2(-1, 0)) {
        return ChunkDirection::West;
    } else if (offset == glm::ivec2(-1, -1)) {
        return ChunkDirection::NorthWest;
    }
}

ChunkManager::ChunkManager(entt::registry& chunkRegistry, FreeCam& freeCam, int32_t viewDistance) {
    m_registry = &chunkRegistry;
    m_freeCam = &freeCam;
    m_viewDistance = viewDistance;
    m_viewDistance2 = viewDistance * viewDistance;
}

void ChunkManager::update(VoxelEngine::Clock& clock) {
    glm::ivec3 worldChunk = Chunk::worldToWorldChunk(m_freeCam->position());
    worldChunk.y = 0;
    glm::ivec2 coord = glm::ivec2(worldChunk.x, worldChunk.z);

    {
        auto it = m_chunkMap.find(coord);
        if (it == m_chunkMap.end()) {
            makeChunkGroup(coord);
        }
    }

    {
        auto it = m_chunkMap.begin();
        while (it != m_chunkMap.end()) {
            if (distance2(it->first, coord) > m_viewDistance2) {
                it = destroyChunkGroup(it, it->first);
            } else {
                it++;
            }
        }
    }

    for (int32_t x = -m_viewDistance; x <= m_viewDistance; x++) {
        for (int32_t y = -m_viewDistance; y <= m_viewDistance; y++) {
            glm::ivec2 neighbor = glm::ivec2(x, y) + coord;
            if (distance2(neighbor, coord) > m_viewDistance2) continue;

            auto it = m_chunkMap.find(neighbor);
            if (it == m_chunkMap.end()) {
                makeChunkGroup(neighbor);
            }
        }
    }
}

ChunkGroup& ChunkManager::makeChunkGroup(glm::ivec2 coord) {
    auto result = m_chunkMap.emplace(coord, std::make_unique<ChunkGroup>(coord, *m_registry));
    ChunkGroup& group = *result.first->second;

    for (auto offset : Chunk::Neighbors8_2D) {
        ChunkDirection dir = ChunkGroup::getDirection(offset);

        auto it = m_chunkMap.find(coord + offset);
        if (it != m_chunkMap.end()) {
            auto& neighbor = *it->second;
            group.setNeighbor(dir, &neighbor);
            neighbor.setNeighbor(ChunkGroup::getOpposite(dir), &group);
        }
    }

    return group;
}

ChunkManager::ChunkMap::iterator ChunkManager::destroyChunkGroup(ChunkMap::iterator it, glm::ivec2 coord) {
    ChunkGroup& group = *it->second;

    for (auto offset : Chunk::Neighbors8_2D) {
        ChunkDirection dir = ChunkGroup::getDirection(offset);

        auto neighborIt = m_chunkMap.find(coord + offset);
        if (neighborIt != m_chunkMap.end()) {
            auto& neighbor = *neighborIt->second;
            neighbor.setNeighbor(ChunkGroup::getOpposite(dir), nullptr);
        }
    }

    return m_chunkMap.erase(it);
}

int32_t ChunkManager::distance2(glm::ivec2 a, glm::ivec2 b) {
    glm::ivec2 diff = a - b;
    return (diff.x * diff.x) + (diff.y * diff.y);
}