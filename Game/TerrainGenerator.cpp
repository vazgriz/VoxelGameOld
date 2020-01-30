#include "TerrainGenerator.h"
#include "World.h"
#include "ChunkManager.h"
#include "Chunk.h"
#include <array>

TerrainGenerator::TerrainGenerator(World& world, ChunkManager& chunkManager) : m_queue(queueSize) {
    m_world = &world;
    m_chunkManager = &chunkManager;
}

void TerrainGenerator::run() {
    m_running = true;
    m_thread = std::thread([this]() {
        loop();
    });
}

void TerrainGenerator::stop() {
    m_running = false;
    m_queue.cancel();
    m_thread.join();
}

bool TerrainGenerator::enqueue(glm::ivec2 coord) {
    return m_queue.tryEnqueue(coord);
}

void TerrainGenerator::loop() {
    while (m_running) {
        glm::ivec2 coord;
        bool valid = m_queue.dequeue(coord);
        if (!valid) return;

        generate(coord);
    }
}

void TerrainGenerator::generate(glm::ivec2 coord) {
    auto lock = m_world->getReadLock();
    auto view = m_world->registry().view<Chunk>();

    for (int32_t i = 0; i < World::worldHeight; i++) {
        glm::ivec3 worldChunkPos = { coord.x, i, coord.y };
        entt::entity entity = m_world->getEntity(worldChunkPos);
        if (!m_world->registry().valid(entity)) return;

        auto& chunk = view.get(entity);

        for (auto pos : Chunk::Positions()) {
            auto& block = chunk.blocks()[pos];
            block.type = 3;
        }

        chunk.setLoadState(ChunkLoadState::Loaded);
    }

    lock.unlock();
    m_chunkManager->generateResultQueue().enqueue(coord);
}