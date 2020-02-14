#include "TerrainGenerator.h"
#include "World.h"
#include "ChunkManager.h"
#include "Chunk.h"
#include <array>

TerrainGenerator::TerrainGenerator(World& world, ChunkManager& chunkManager) : m_queue(queueSize) {
    m_world = &world;
    m_chunkManager = &chunkManager;

    m_baseNoise.SetSeed(0);
    m_baseNoise.SetFrequency(0.005f);

    m_caveNoise1.SetSeed(1);
    m_caveNoise1.SetFractalType(FastNoise::FractalType::RigidMulti);
    m_caveNoise1.SetFrequency(0.01f);
    m_caveNoise2.SetSeed(2);
    m_caveNoise2.SetFractalType(FastNoise::FractalType::RigidMulti);
    m_caveNoise2.SetFrequency(0.01f);
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

int32_t seaLevel = 64;
int32_t amplitude = 32;
int32_t dirtDepth = 3;
int32_t caveAttenuationDepth = 10;
float caveAttenuation = 0.75f;

void TerrainGenerator::generate(glm::ivec2 coord) {
    std::array<std::array<int32_t, Chunk::chunkSize>, Chunk::chunkSize> values;
    std::array<ChunkData<bool, Chunk::chunkSize>, World::worldHeight> caveValues;

    for (int32_t x = 0; x < Chunk::chunkSize; x++) {
        for (int32_t y = 0; y < Chunk::chunkSize; y++) {
            glm::vec2 pos = coord * Chunk::chunkSize + glm::ivec2(x, y);
            values[x][y] = static_cast<int32_t>(round(m_baseNoise.GetSimplexFractal(pos.x, pos.y) * amplitude + seaLevel));
        }
    }

    for (int32_t i = 0; i < World::worldHeight; i++) {
        glm::ivec3 worldChunkRoot = glm::ivec3(coord.x, i, coord.y) * Chunk::chunkSize;

        for (auto pos : Chunk::Positions()) {
            glm::ivec3 worldPos = worldChunkRoot + pos;
            int32_t ground = values[pos.x][pos.z];

            if (worldPos.y > ground) {
                caveValues[i][pos] = false;
                continue;
            }

            float caveValue1 = m_caveNoise1.GetSimplexFractal(worldPos.x, worldPos.y, worldPos.z);
            float caveValue2 = m_caveNoise2.GetSimplexFractal(worldPos.x, worldPos.y, worldPos.z);

            float caveValue = caveValue1 * caveValue2;

            if (worldPos.y <= ground && worldPos.y > ground - caveAttenuationDepth) {
                float diff = ground - worldPos.y;
                float factor = diff / caveAttenuationDepth;
                caveValue *= caveAttenuation + (factor * (1 - caveAttenuation));
            }

            caveValues[i][pos] = caveValue > 0.125f;
        }
    }

    TerrainResults results = {};
    results.coord = coord;

    for (int32_t i = 0; i < World::worldHeight; i++) {
        glm::ivec3 worldChunkPos = { coord.x, i, coord.y };
        for (auto pos : Chunk::Positions()) {
            auto& block = results.blocks[i][pos];
            glm::ivec3 worldPos = worldChunkPos * Chunk::chunkSize + pos;

            int32_t ground = values[pos.x][pos.z];
            bool caveValue = caveValues[i][pos];

            if (worldPos.y > ground) {
                block.type = 1;
                continue;
            }

            if (caveValue) {
                block.type = 1;
            } else if (worldPos.y == ground) {
                block.type = 3;
            } else if (worldPos.y < ground && worldPos.y >= ground - dirtDepth) {
                block.type = 2;
            } else if (worldPos.y < ground - dirtDepth) {
                block.type = 4;
            }
        }
    }

    auto lock = m_world->getReadLock();
    std::array<Chunk*, World::worldHeight> chunks;
    auto view = m_world->registry().view<Chunk>();

    for (int32_t i = 0; i < World::worldHeight; i++) {
        glm::ivec3 worldChunkPos = { coord.x, i, coord.y };
        entt::entity entity = m_world->getEntity(worldChunkPos);
        if (!m_world->registry().valid(entity)) return;
        chunks[i] = &view.get(entity);
    }

    for (int32_t x = 0; x < Chunk::chunkSize; x++) {
        for (int32_t z = 0; z < Chunk::chunkSize; z++) {
            for (int32_t y = (Chunk::chunkSize * World::worldHeight) - 1; y >= 0; y--) {
                auto result = Chunk::divide(y, Chunk::chunkSize);
                glm::ivec3 pos = { x, result[1], z };

                if (results.blocks[result[0]][pos].type > 1) break;
                chunks[result[0]]->getLightUpdates().enqueue({ Light(15), pos });
            }

            for (int32_t y = 0; y < Chunk::chunkSize * World::worldHeight; y++) {
                auto result = Chunk::divide(y, Chunk::chunkSize);
                glm::ivec3 pos = { x, result[1], z };

                if (results.blocks[result[0]][pos].type > 1) break;
                chunks[result[0]]->getLightUpdates().enqueue({ Light(15), pos });
            }
        }
    }

    lock.unlock();
    m_chunkManager->generateResultQueue().enqueue(results);
}