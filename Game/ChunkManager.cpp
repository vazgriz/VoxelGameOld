#include "ChunkManager.h"
#include "ChunkMesh.h"
#include "TerrainGenerator.h"
#include "ChunkUpdater.h"

ChunkGroup::ChunkGroup(glm::ivec2 coord, World& world) : m_neighbors() {
    m_coord = coord;
    m_world = &world;
    m_loadState = ChunkLoadState::Loading;

    for (int32_t i = 0; i < World::worldHeight; i++) {
        entt::entity chunkEntity = m_world->createChunk(glm::ivec3(coord.x, i, coord.y));
        m_chunks.push_back(chunkEntity);
    }
}

ChunkGroup::~ChunkGroup() {
    for (int32_t i = 0; i < m_chunks.size(); i++) {
        m_world->destroyChunk(glm::ivec3(m_coord.x, i, m_coord.y), m_chunks[i]);
    }
}

void ChunkGroup::setLoadState(ChunkLoadState loadState) {
    m_loadState = loadState;

    auto view = m_world->registry().view<Chunk>();
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

ChunkManager::ChunkManager(World& world, FreeCam& freeCam, int32_t viewDistance) {
    m_world = &world;
    m_freeCam = &freeCam;
    m_viewDistance = viewDistance;
    m_viewDistance2 = viewDistance * viewDistance;

    m_lastPos = { -1, -1, -1 };
}

void ChunkManager::setTerrainGenerator(TerrainGenerator& terrainGenerator) {
    m_terrainGenerator = &terrainGenerator;
}

void ChunkManager::setChunkUpdater(ChunkUpdater& chunkUpdater) {
    m_chunkUpdater = &chunkUpdater;
}

void ChunkManager::update(VoxelEngine::Clock& clock) {
    glm::ivec3 worldChunk = Chunk::worldToWorldChunk(m_freeCam->position());
    glm::ivec2 coord = { worldChunk.x, worldChunk.z };

    worldChunk.y = std::clamp<int32_t>(worldChunk.y, 0, World::worldHeight);

    if (worldChunk != m_lastPos) {
        m_lastPos = worldChunk;
        auto lock = m_world->getWriteLock();

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

    glm::ivec3 worldChunk2D = worldChunk;
    worldChunk2D.y = 0;
    m_generateQueue.update(worldChunk2D);

    while (m_generateQueue.count() > 0) {
        auto item = m_generateQueue.peek();
        if (!m_terrainGenerator->enqueue({ item.x, item.z })) break;
        m_generateQueue.dequeue();
    }

    auto& generateResults = m_generateResultQueue.swapDequeue();
    auto view = m_world->registry().view<Chunk, ChunkMesh>();

    while (generateResults.size() > 0) {
        auto results = generateResults.front();
        auto coord = results.coord;
        generateResults.pop();

        auto it = m_chunkMap.find(coord);
        if (it == m_chunkMap.end()) continue;

        auto& group = it->second;
        group.setLoadState(ChunkLoadState::Loaded);

        for (int32_t i = 0; i < World::worldHeight; i++) {
            glm::ivec3 worldChunkPos = { coord.x, i, coord.y };
            auto chunkEntity = group.chunks()[i];
            auto& chunk = view.get<Chunk>(chunkEntity);
            auto& chunkMesh = view.get<ChunkMesh>(chunkEntity);

            memcpy(chunk.blocks().data(), results.blocks[i].data(), sizeof(ChunkData<Block, Chunk::chunkSize>));
            memset(chunk.light().data(), 0, sizeof(ChunkData<Light, Chunk::chunkSize>));

            chunkMesh.clearMesh();
            m_updateQueue.enqueue(worldChunkPos);
        }
    }

    m_updateQueue.update(worldChunk);

    while (m_updateQueue.count() > 0) {
        auto item = m_updateQueue.peek();
        if (!m_world->valid(item)) {
            m_updateQueue.dequeue();
            continue;
        }

        auto& chunk = view.get<Chunk>(m_world->getEntity(item));

        if (chunk.loadState() != ChunkLoadState::Loaded) {
            m_updateRequeue.push(item);
            m_updateQueue.dequeue();
            continue;
        }

        bool skip = false;
        for (auto offset : Chunk::Neighbors26) {
            auto neighborEntity = chunk.neighbor(offset);

            if (neighborEntity != entt::null) {
                auto& neighborChunk = view.get<Chunk>(neighborEntity);
                if (neighborChunk.loadState() != ChunkLoadState::Loaded) {
                    m_updateRequeue.push(item);
                    m_updateQueue.dequeue();
                    skip = true;
                    break;
                }
            }
        }

        if (skip) {
            continue;
        }

        if (!m_chunkUpdater->queue(item)) break;
        m_updateQueue.dequeue();
    }

    while (m_updateRequeue.size() > 0) {
        auto& item = m_updateRequeue.front();
        m_updateQueue.enqueue(item);
        m_updateRequeue.pop();
    }

    auto& worldUpdates = m_world->getUpdates();

    while (worldUpdates.size() > 0) {
        auto update = worldUpdates.front();
        m_updateQueue.enqueue(update);
        worldUpdates.pop();
    }
}

ChunkGroup& ChunkManager::makeChunkGroup(glm::ivec2 coord) {
    auto result = m_chunkMap.emplace(coord, ChunkGroup(coord, *m_world));
    ChunkGroup& group = result.first->second;
    group.setLoadState(ChunkLoadState::Loading);

    for (auto offset : Chunk::Neighbors8_2D) {
        ChunkDirection dir = ChunkGroup::getDirection(offset);

        auto it = m_chunkMap.find(coord + offset);
        if (it != m_chunkMap.end()) {
            auto& neighbor = it->second;
            group.setNeighbor(dir, &neighbor);
            neighbor.setNeighbor(ChunkGroup::getOpposite(dir), &group);

            auto view = m_world->registry().view<Chunk>();
        }
    }

    auto view = m_world->registry().view<Chunk>();

    for (size_t i = 0; i < World::worldHeight; i++) {
        glm::ivec3 worldChunkPos = glm::ivec3(coord.x, i, coord.y);
        auto chunkEntity = m_world->getEntity(worldChunkPos);
        auto& chunk = view.get(chunkEntity);

        for (auto offset : Chunk::Neighbors26) {
            auto neighborEntity = m_world->getEntity(worldChunkPos + offset);
            chunk.setNeighbor(offset, neighborEntity);

            if (neighborEntity != entt::null) {
                auto& neighbor = view.get(neighborEntity);
                neighbor.setNeighbor(-offset, chunkEntity);
            }
        }
    }

    m_generateQueue.enqueue({ coord.x, 0, coord.y });

    return group;
}

ChunkManager::ChunkMap::iterator ChunkManager::destroyChunkGroup(ChunkMap::iterator it, glm::ivec2 coord) {
    ChunkGroup& group = it->second;

    group.setLoadState(ChunkLoadState::Unloaded);

    for (auto offset : Chunk::Neighbors8_2D) {
        ChunkDirection dir = ChunkGroup::getDirection(offset);

        auto neighborIt = m_chunkMap.find(coord + offset);
        if (neighborIt != m_chunkMap.end()) {
            auto& neighbor = neighborIt->second;
            neighbor.setNeighbor(ChunkGroup::getOpposite(dir), nullptr);
        }
    }

    auto view = m_world->registry().view<Chunk>();

    for (size_t i = 0; i < World::worldHeight; i++) {
        glm::ivec3 worldChunkPos = glm::ivec3(coord.x, i, coord.y);
        auto chunkEntity = m_world->getEntity(worldChunkPos);
        auto& chunk = view.get(chunkEntity);

        for (auto offset : Chunk::Neighbors26) {
            auto neighborEntity = m_world->getEntity(worldChunkPos + offset);

            if (neighborEntity != entt::null) {
                auto& neighbor = view.get(neighborEntity);
                neighbor.setNeighbor(-offset, entt::null);
            }
        }
    }

    m_generateQueue.remove({ coord.x, 0, coord.y });

    for (int32_t i = 0; i < worldHeight; i++) {
        m_updateQueue.remove({ coord.x, i, coord.y });
    }

    return m_chunkMap.erase(it);
}

int32_t ChunkManager::distance2(glm::ivec2 a, glm::ivec2 b) {
    glm::ivec2 diff = a - b;
    return (diff.x * diff.x) + (diff.y * diff.y);
}