#include "ChunkUpdater.h"

ChunkUpdater::ChunkUpdater(VoxelEngine::Engine& engine, entt::registry& registry, BlockManager& blockManager) {
    m_engine = &engine;
    m_registry = &registry;
    m_blockManager = &blockManager;

    createIndexBuffer();
}

void ChunkUpdater::setTransferNode(VoxelEngine::TransferNode& transferNode) {
    m_transferNode = &transferNode;

    m_transferNode->transfer(m_indexBuffer, m_indexBufferSize, 0, m_indexData.data());
    m_indexData = {};
}

void ChunkUpdater::update(VoxelEngine::Clock& clock) {
    auto view = m_registry->view<Chunk, ChunkMesh>();

    for (auto entity : view) {
        Chunk& chunk = view.get<Chunk>(entity);
        if (chunk.activeState() != ChunkActiveState::Active) continue;

        ChunkMesh& chunkMesh = view.get<ChunkMesh>(entity);

        if (chunkMesh.mesh().bindingCount() == 0) {
            uint32_t indexCount = makeMesh(chunk, chunkMesh);
            transferMesh(chunkMesh, indexCount);
        }
    }
}

void ChunkUpdater::createIndexBuffer() {
    uint32_t index = 0;

    for (uint32_t i = 0; i < 2048; i++) {
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

uint32_t ChunkUpdater::makeMesh(Chunk& chunk, ChunkMesh& chunkMesh) {
    m_vertexData.clear();
    m_colorData.clear();
    m_uvData.clear();

    uint32_t indexCount = 0;

    for (glm::ivec3 pos : Chunk::Positions()) {
        Block block = chunk.blocks()[pos];
        if (block.type == 0) continue;
        if (block.type == 1) continue;

        for (size_t i = 0; i < Chunk::Neighbors6.size(); i++) {
            glm::ivec3 offset = Chunk::Neighbors6[i];
            glm::ivec3 neighborPos = pos + offset;
            BlockType& blockType = m_blockManager->getType(block.type);

            if (((neighborPos.x < 0 || neighborPos.x >= Chunk::chunkSize)
                || (neighborPos.y < 0 || neighborPos.y >= Chunk::chunkSize)
                || (neighborPos.z < 0 || neighborPos.z >= Chunk::chunkSize))
                || chunk.blocks()[neighborPos].type == 1)
            {
                Chunk::FaceArray& faceArray = Chunk::NeighborFaces[i];
                size_t faceIndex = blockType.getFaceIndex(i);

                for (size_t j = 0; j < faceArray.size(); j++) {
                    m_vertexData.push_back(glm::i8vec4(pos + faceArray[j], 0));
                    m_colorData.push_back(glm::i8vec4(pos.x * 16, pos.y * 16, pos.z * 16, 0));

                    m_uvData.push_back(glm::i8vec4(Chunk::uvFaces[j], static_cast<uint8_t>(faceIndex), 0));
                }

                indexCount++;
            }
        }
    }

    return indexCount * 6;
}

void ChunkUpdater::transferMesh(ChunkMesh& chunkMesh, uint32_t indexCount) {
    if (m_vertexData.size() == 0) return;
    size_t vertexSize = m_vertexData.size() * sizeof(glm::ivec3);
    size_t colorSize = m_colorData.size() * sizeof(glm::i8vec4);
    size_t uvSize = m_uvData.size() * sizeof(glm::ivec3);

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
        chunkMesh.mesh().setIndexCount(indexCount);
    }

    m_transferNode->transfer(chunkMesh.mesh().getBinding(0), vertexSize, 0, m_vertexData.data());
    m_transferNode->transfer(chunkMesh.mesh().getBinding(1), colorSize, 0, m_colorData.data());
    m_transferNode->transfer(chunkMesh.mesh().getBinding(2), uvSize, 0, m_uvData.data());

    chunkMesh.setDirty();
}