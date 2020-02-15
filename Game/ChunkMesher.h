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

struct MeshUpdate2 {
    size_t index;
    glm::ivec3 coord;
    ChunkData<Light, Chunk::chunkSize + 2> lightBuffer;
};

class ChunkMesher : public VoxelEngine::System {
    static const size_t queueSize = 16;
public:
    ChunkMesher(VoxelEngine::Engine& engine, World& world, BlockManager& blockManager);

    void setTransferNode(VoxelEngine::TransferNode& transferNode);

    void update(VoxelEngine::Clock& clock);

    void run();
    void stop();

    bool queue(glm::ivec3 coord);

private:
    using ChunkBuffer = ChunkData<Block, Chunk::chunkSize + 2>;
    using LightBuffer = ChunkData<Light, Chunk::chunkSize + 2>;

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
    VoxelEngine::BlockingQueue<glm::ivec3> m_requestQueue;
    VoxelEngine::BufferedQueue<MeshUpdate2> m_resultQueue;

    void createIndexBuffer();

    size_t makeMesh(glm::ivec3 worldChunkPos, ChunkBuffer& chunkBuffer, LightBuffer& lightBuffer);
    void transferMesh(ChunkMesh& chunkMesh, size_t index);

    void update(glm::ivec3 worldChunkPos);

    void loop();
};