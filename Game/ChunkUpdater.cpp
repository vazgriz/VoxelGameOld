#include "ChunkUpdater.h"
#include "Chunk.h"
#include "ChunkMesh.h"

ChunkUpdater::ChunkUpdater(VoxelEngine::Engine& engine, World& world, BlockManager& blockManager) : m_requestQueue(queueSize) {
    m_engine = &engine;
    m_world = &world;
    m_blockManager = &blockManager;

    createIndexBuffer();
}

void ChunkUpdater::setTransferNode(VoxelEngine::TransferNode& transferNode) {
    m_transferNode = &transferNode;

    m_transferNode->transfer(*m_indexBuffer, m_indexBufferSize, 0, m_indexData.data());
    m_indexData = {};
}

void ChunkUpdater::update(VoxelEngine::Clock& clock) {
    std::queue<MeshUpdate2>& queue = m_resultQueue.swapDequeue();
    auto view = m_world->registry().view<ChunkMesh>();

    while (queue.size() > 0) {
        auto& update = queue.front();
        if (m_world->registry().valid(update.entity)) {
            transferMesh(view.get(update.entity), update.index);
        }
        queue.pop();
    }
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

bool ChunkUpdater::queue(entt::entity entity) {
    return m_requestQueue.tryEnqueue(entity);
}

void ChunkUpdater::loop() {
    while (m_running) {
        entt::entity entity;
        bool valid = m_requestQueue.dequeue(entity);
        if (!valid) return;

        auto lock = m_world->getReadLock();
        if (!m_world->registry().valid(entity)) continue;

        auto view = m_world->registry().view<Chunk, ChunkMesh>();
        Chunk& chunk = view.get<Chunk>(entity);
        if (chunk.loadState() != ChunkLoadState::Loaded) continue;

        for (auto offset : Chunk::Neighbors26) {
            auto neighborEntity = chunk.neighbor(offset);
            if (neighborEntity == entt::null) continue;
        }

        ChunkMesh& chunkMesh = view.get<ChunkMesh>(entity);

        size_t updateIndex = makeMesh(chunk, chunkMesh);
        m_resultQueue.enqueue({ updateIndex, entity });
    }
}

void ChunkUpdater::createIndexBuffer() {
    uint32_t index = 0;

    for (uint32_t i = 0; i < 2048 * 6; i++) {
        m_indexData.push_back(index + 0);
        m_indexData.push_back(index + 1);
        m_indexData.push_back(index + 2);
        m_indexData.push_back(index + 1);
        m_indexData.push_back(index + 3);
        m_indexData.push_back(index + 2);
        index += 4;
    }

    m_indexCount = static_cast<uint32_t>(m_indexData.size());
    m_indexBufferSize = m_indexData.size() * sizeof(uint32_t);

    vk::BufferCreateInfo info = {};
    info.size = m_indexBufferSize;
    info.usage = vk::BufferUsageFlags::IndexBuffer | vk::BufferUsageFlags::TransferDst;
    info.sharingMode = vk::SharingMode::Exclusive;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_indexBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, info, allocInfo);
}

size_t ChunkUpdater::makeMesh(Chunk& chunk, ChunkMesh& chunkMesh) {
    size_t index = m_updateIndex;
    m_updateIndex = (m_updateIndex + 1) % (queueSize * 2);

    MeshUpdate& update = m_updates[index];
    update.vertexData.clear();
    update.colorData.clear();
    update.uvData.clear();

    uint32_t indexCount = 0;

    auto view = m_world->registry().view<Chunk>();

    for (glm::ivec3 pos : Chunk::Positions()) {
        Block block = chunk.blocks()[pos];
        if (block.type == 0) continue;
        if (block.type == 1) continue;
        BlockType& blockType = m_blockManager->getType(block.type);

        for (size_t i = 0; i < Chunk::Neighbors6.size(); i++) {
            glm::ivec3 offset = Chunk::Neighbors6[i];
            glm::ivec3 neighborPos = pos + offset;
            auto neighborResults = Chunk::split(neighborPos);
            glm::ivec3 neighborOffset = neighborResults[0];
            glm::ivec3 neighborPosMod = neighborResults[1];

            bool visible = false;

            if (neighborOffset == glm::ivec3()) {
                visible = chunk.blocks()[neighborPosMod].type == 1;
            } else {
                entt::entity neighborEntity = chunk.neighbor(neighborOffset);

                if (m_world->registry().valid(neighborEntity)) {
                    auto& neighbor = view.get(neighborEntity);
                    visible = neighbor.blocks()[neighborPosMod].type == 1;
                }
            }

            if (visible) {
                Chunk::FaceArray& faceArray = Chunk::NeighborFaces[i];
                size_t faceIndex = blockType.getFaceIndex(i);

                for (size_t j = 0; j < faceArray.size(); j++) {
                    update.vertexData.push_back(glm::i8vec4(pos + faceArray[j], 0));
                    update.colorData.push_back(glm::i8vec4(pos.x * 16, pos.y * 16, pos.z * 16, 0));
                    update.uvData.push_back(glm::i8vec4(Chunk::uvFaces[j], static_cast<uint8_t>(faceIndex), 0));
                }

                indexCount++;
            }
        }
    }

    update.indexCount = indexCount * 6;

    return index;
}

void ChunkUpdater::transferMesh(ChunkMesh& chunkMesh, size_t index) {
    MeshUpdate& update = m_updates[index];

    if (update.indexCount == 0) {
        chunkMesh.mesh().setIndexCount(0);
        chunkMesh.mesh().setIndexBuffer(nullptr, vk::IndexType::Uint32, 0);
        return;
    }

    size_t vertexSize = update.vertexData.size() * sizeof(glm::i8vec4);
    size_t colorSize = update.colorData.size() * sizeof(glm::i8vec4);
    size_t uvSize = update.uvData.size() * sizeof(glm::i8vec4);

    if (chunkMesh.mesh().bindingCount() == 0 || chunkMesh.mesh().getBinding(0)->size() < vertexSize) {
        vk::BufferCreateInfo vertexInfo = {};
        vertexInfo.size = vertexSize;
        vertexInfo.usage = vk::BufferUsageFlags::VertexBuffer | vk::BufferUsageFlags::TransferDst;
        vertexInfo.sharingMode = vk::SharingMode::Exclusive;

        vk::BufferCreateInfo colorInfo = vertexInfo;
        colorInfo.size = colorSize;

        vk::BufferCreateInfo uvInfo = vertexInfo;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        std::shared_ptr<VoxelEngine::Buffer> vertexBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, vertexInfo, allocInfo);
        std::shared_ptr<VoxelEngine::Buffer> colorBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, colorInfo, allocInfo);
        std::shared_ptr<VoxelEngine::Buffer> uvBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, uvInfo, allocInfo);

        chunkMesh.mesh().clearBindings();
        chunkMesh.mesh().addBinding(vertexBuffer);
        chunkMesh.mesh().addBinding(colorBuffer);
        chunkMesh.mesh().addBinding(uvBuffer);

        chunkMesh.mesh().setIndexBuffer(m_indexBuffer, vk::IndexType::Uint32, 0);
        chunkMesh.mesh().setIndexCount(update.indexCount);
    }

    m_transferNode->transfer(*chunkMesh.mesh().getBinding(0), vertexSize, 0, update.vertexData.data());
    m_transferNode->transfer(*chunkMesh.mesh().getBinding(1), colorSize, 0, update.colorData.data());
    m_transferNode->transfer(*chunkMesh.mesh().getBinding(2), uvSize, 0, update.uvData.data());

    chunkMesh.setDirty();
}