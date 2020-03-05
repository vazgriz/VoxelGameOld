#pragma once
#include <Engine/Engine.h>
#include <Engine/BlockingQueue.h>
#include <Engine/BufferedQueue.h>
#include <entt/entt.hpp>
#include "Chunk.h"
#include "World.h"
#include "BlockManager.h"
#include "ChunkManager.h"

struct UpdateResults {
    glm::ivec3 worldChunkPos;
    ChunkData<Block, Chunk::chunkSize + 2> blockBuffer;
    ChunkData<Light, Chunk::chunkSize + 2> lightBuffer;
};

class ChunkUpdater {
public:
    static const size_t queueSize = 16;
    ChunkUpdater(VoxelEngine::Engine& engine, World& world, BlockManager& blockManager, ChunkManager& chunkManager);

    void run();
    void stop();

    bool queue(glm::ivec3 coord);

private:
    using ChunkBuffer = ChunkData<Block, Chunk::chunkSize + 2>;
    using LightBuffer = ChunkData<Light, Chunk::chunkSize + 2>;

    VoxelEngine::Engine* m_engine;
    World* m_world;
    BlockManager* m_blockManager;
    ChunkManager* m_chunkManager;

    bool m_running = false;
    std::thread m_thread;

    VoxelEngine::BlockingQueue<glm::ivec3> m_requestQueue;

    void update(glm::ivec3 worldChunkPos);
    void updateLight(std::queue<LightUpdate>& queue, ChunkBuffer& chunkBuffer, LightBuffer& lightBuffer, ChunkData<Chunk*, 3>& neighborChunks);

    void loop();
};