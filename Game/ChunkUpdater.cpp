#include "ChunkUpdater.h"
#include "Chunk.h"
#include "ChunkMesh.h"

ChunkUpdater::ChunkUpdater(VoxelEngine::Engine& engine, World& world, BlockManager& blockManager, ChunkManager& chunkManager) : m_requestQueue(queueSize) {
    m_engine = &engine;
    m_world = &world;
    m_blockManager = &blockManager;
    m_chunkManager = &chunkManager;
}

void ChunkUpdater::run() {
    m_running = true;
    m_thread = std::thread([this] { loop(); });
}

void ChunkUpdater::stop() {
    m_running = false;
    m_requestQueue.cancel();
    m_thread.join();
}

bool ChunkUpdater::queue(glm::ivec3 coord) {
    return m_requestQueue.tryEnqueue(coord);
}

void ChunkUpdater::loop() {
    while (m_running) {
        glm::ivec3 coord;
        bool valid = m_requestQueue.dequeue(coord);
        if (!valid) return;

        update(coord);
    }
}

void ChunkUpdater::update(glm::ivec3 worldChunkPos) {
    ChunkBuffer blocks;
    LightBuffer light;
    ChunkData<Chunk*, 3> neighborChunks;
    const glm::ivec3 root = { 1, 1, 1 };
    std::queue<LightUpdate> queue;

    {
        auto lock = m_world->getLock();
        entt::entity entity = m_world->getEntity(worldChunkPos);
        if (entity == entt::null) return;

        auto view = m_world->registry().view<Chunk, ChunkMesh>();
        Chunk& chunk = view.get<Chunk>(entity);
        ChunkMesh& chunkMesh = view.get<ChunkMesh>(entity);

        for (auto offset : Chunk::Neighbors26) {
            entt::entity neighborEntity = chunk.neighbor(offset);

            if (m_world->valid(neighborEntity)) {
                auto& neighbor = view.get<Chunk>(neighborEntity);
                neighborChunks[root + offset] = &neighbor;
            } else {
                neighborChunks[root + offset] = nullptr;
            }
        }

        for (int32_t x = -1; x < Chunk::chunkSize + 1; x++) {
            for (int32_t y = -1; y < Chunk::chunkSize + 1; y++) {
                for (int32_t z = -1; z < Chunk::chunkSize + 1; z++) {
                    glm::ivec3 pos = { x, y, z };
                    auto results = Chunk::split(pos);
                    glm::ivec3 chunkOffset = results[0];
                    glm::ivec3 posMod = results[1];

                    if (chunkOffset == glm::ivec3()) {
                        blocks[root + pos] = chunk.blocks()[posMod];
                        light[root + pos] = chunk.light()[posMod];
                    } else {
                        auto neighborChunk = neighborChunks[root + chunkOffset];

                        if (neighborChunk != nullptr) {
                            blocks[root + pos] = neighborChunk->blocks()[posMod];
                            light[root + pos] = neighborChunk->light()[posMod];
                        } else {
                            blocks[root + pos] = Block(0);
                            light[root + pos] = Light(15);
                        }
                    }
                }
            }
        }

        auto& lightUpdates = chunk.getLightUpdates();

        while (lightUpdates.size() > 0) {
            auto update = lightUpdates.front();
            lightUpdates.pop();
            queue.push(update);
        }
    }

    updateLight(queue, blocks, light, neighborChunks);

    m_chunkManager->updateResultQueue().enqueue({ worldChunkPos, light });
}

void ChunkUpdater::updateLight(std::queue<LightUpdate>& queue, ChunkBuffer& chunkBuffer, LightBuffer& lightBuffer, ChunkData<Chunk*, 3>& neighborChunks) {
    const glm::ivec3 root = { 1, 1, 1 };

    while (queue.size() > 0) {
        auto light = queue.front().light;
        auto pos = queue.front().inChunkPos;
        queue.pop();

        if (!(lightBuffer[root + pos] < light)) continue;

        lightBuffer[root + pos].overwrite(light);

        for (auto offset : Chunk::Neighbors6) {
            glm::ivec3 neighborPos = pos + offset;
            auto neighborResults = Chunk::split(neighborPos);
            glm::ivec3 neighborChunkOffset = neighborResults[0];
            glm::ivec3 neighborPosMod = neighborResults[1];

            int32_t loss = 1;

            if (offset == glm::ivec3(0, 1, 0) || offset == glm::ivec3(0, -1, 0)) {
                loss = 0;
            }

            Light newLight(std::max<int8_t>(0, light.sun - loss));

            if (neighborChunkOffset == glm::ivec3()) {
                Block& block = chunkBuffer[root + neighborPosMod];

                if (block.type > 1) continue;

                Light& currentLight = lightBuffer[root + neighborPosMod];
                if (newLight > currentLight) {
                    queue.push({ newLight, neighborPosMod });
                }
            } else {
                Chunk* neighborChunk = neighborChunks[root + neighborChunkOffset];
                
                if (neighborChunk != nullptr) {
                    neighborChunk->queueLightUpdate({ newLight, neighborPosMod });
                }
            }
        }
    }
}