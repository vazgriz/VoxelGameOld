#pragma once
#include <Engine/Engine.h>
#include <Engine/RenderGraph/TransferNode.h>
#include <Engine/BlockingQueue.h>
#include <Engine/BufferedQueue.h>
#include <entt/entt.hpp>
#include "Chunk.h"
#include "BlockManager.h"
#include "World.h"

struct MeshUpdate {
    std::vector<glm::i8vec4> vertexData;
    std::vector<glm::i8vec4> colorData;
    std::vector<glm::i8vec4> uvData;
    uint32_t indexCount;
};

struct MeshUpdate2{
    size_t index;
    entt::entity entity;
};

class ChunkUpdater : public VoxelEngine::System {
public:
    static const size_t queueSize = 16;
    ChunkUpdater(VoxelEngine::Engine& engine, World& world, BlockManager& blockManager);

    void setTransferNode(VoxelEngine::TransferNode& transferNode);

    void update(VoxelEngine::Clock& clock);

    void run();
    void stop();

    bool queue(entt::entity);

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::TransferNode* m_transferNode;
    BlockManager* m_blockManager;
    World* m_world;

    bool m_running = false;
    std::thread m_thread;

    std::vector<uint32_t> m_indexData;
    size_t m_indexBufferSize;
    uint32_t m_indexCount;
    std::shared_ptr<VoxelEngine::Buffer> m_indexBuffer;

    std::array<MeshUpdate, queueSize * 2> m_updates;
    size_t m_updateIndex = 0;
    VoxelEngine::BlockingQueue<entt::entity> m_requestQueue;
    VoxelEngine::BufferedQueue<MeshUpdate2> m_resultQueue;

    void createIndexBuffer();
    size_t makeMesh(Chunk& chunk, ChunkMesh& chunkMesh);
    void transferMesh(ChunkMesh& chunkMesh, size_t index);

    void loop();
};