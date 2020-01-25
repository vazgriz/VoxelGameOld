#include "ChunkUpdater.h"

ChunkUpdater::ChunkUpdater(uint32_t priority, VoxelEngine::Engine& engine, entt::registry& registry) : VoxelEngine::System(priority) {
    m_engine = &engine;
    m_registry = &registry;
}

void ChunkUpdater::setTransferNode(VoxelEngine::TransferNode& transferNode) {
    m_transferNode = &transferNode;
}

void ChunkUpdater::update(VoxelEngine::Clock& clock) {
    auto view = m_registry->view<Chunk, ChunkMesh>();

    for (auto entity : view) {
        Chunk& chunk = view.get<Chunk>(entity);
        ChunkMesh& chunkMesh = view.get<ChunkMesh>(entity);
        makeMesh(chunk, chunkMesh);
        transferMesh(chunkMesh);
    }
}

void ChunkUpdater::makeMesh(Chunk& chunk, ChunkMesh& chunkMesh) {
    m_vertexData.clear();
    m_colorData.clear();
    m_indexData.clear();

    uint32_t index = 0;

    for (glm::ivec3 pos : Chunk::Positions()) {
        Block block = chunk.blocks()[pos];
        if (block.type == 0) continue;

        for (size_t i = 0; i < Chunk::Neighbors6.size(); i++) {
            glm::ivec3 offset = Chunk::Neighbors6[i];
            glm::ivec3 neighborPos = pos + offset;

            if (((neighborPos.x < 0 || neighborPos.x >= Chunk::chunkSize)
                || (neighborPos.y < 0 || neighborPos.y >= Chunk::chunkSize)
                || (neighborPos.z < 0 || neighborPos.z >= Chunk::chunkSize))
                || chunk.blocks()[neighborPos].type == 0)
            {
                Chunk::FaceArray& faceArray = Chunk::NeighborFaces[i];
                for (size_t j = 0; j < faceArray.size(); j++) {
                    m_vertexData.push_back(pos + faceArray[j]);
                    m_colorData.push_back(glm::i8vec4(pos.x * 16, pos.y * 16, pos.z * 16, 0));
                }

                m_indexData.push_back(index + 0);
                m_indexData.push_back(index + 1);
                m_indexData.push_back(index + 2);
                m_indexData.push_back(index + 1);
                m_indexData.push_back(index + 3);
                m_indexData.push_back(index + 2);
                index += 4;
            }
        }
    }
}

void ChunkUpdater::transferMesh(ChunkMesh& chunkMesh) {
    if (m_vertexData.size() == 0) return;
    size_t vertexSize = m_vertexData.size() * sizeof(glm::ivec3);
    size_t colorSize = m_colorData.size() * sizeof(glm::i8vec4);
    size_t indexSize = m_indexData.size() * sizeof(uint32_t);

    if (chunkMesh.mesh().bindingCount() == 0 || chunkMesh.mesh().getBinding(0)->size() < vertexSize) {
        vk::BufferCreateInfo vertexInfo = {};
        vertexInfo.size = vertexSize;
        vertexInfo.usage = vk::BufferUsageFlags::VertexBuffer | vk::BufferUsageFlags::TransferDst;
        vertexInfo.sharingMode = vk::SharingMode::Exclusive;

        vk::BufferCreateInfo colorInfo = vertexInfo;
        colorInfo.size = colorSize;

        vk::BufferCreateInfo indexInfo = vertexInfo;
        indexInfo.size = indexSize;
        indexInfo.usage = vk::BufferUsageFlags::IndexBuffer | vk::BufferUsageFlags::TransferDst;;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.flags = VMA_MEMORY_USAGE_GPU_ONLY;
        allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        std::shared_ptr<VoxelEngine::Buffer> vertexBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, vertexInfo, allocInfo);
        std::shared_ptr<VoxelEngine::Buffer> colorBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, colorInfo, allocInfo);
        std::shared_ptr<VoxelEngine::Buffer> indexBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, indexInfo, allocInfo);

        chunkMesh.mesh().clearBindings();
        chunkMesh.mesh().addBinding(vertexBuffer);
        chunkMesh.mesh().addBinding(colorBuffer);

        chunkMesh.mesh().setIndexBuffer(indexBuffer, vk::IndexType::Uint32, 0);
        chunkMesh.mesh().setIndexCount(m_indexData.size());
    }

    m_transferNode->transfer(chunkMesh.mesh().getBinding(0), vertexSize, 0, m_vertexData.data());
    m_transferNode->transfer(chunkMesh.mesh().getBinding(1), colorSize, 0, m_colorData.data());
    m_transferNode->transfer(chunkMesh.mesh().indexBuffer(), indexSize, 0, m_indexData.data());

    chunkMesh.setDirty();
}