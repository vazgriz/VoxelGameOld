#include "MeshManager.h"

MeshManager::MeshManager(VoxelEngine::Engine& engine) {
    m_engine = &engine;
}

void MeshManager::setTransferNode(VoxelEngine::TransferNode& transferNode) {
    m_transferNode = &transferNode;
    std::vector<uint32_t> indexData;

    createIndexBuffer(indexData);

    m_transferNode->transfer(*m_indexBuffer, m_indexBufferSize, 0, indexData.data());
}

void MeshManager::createIndexBuffer(std::vector<uint32_t>& indexData) {
    uint32_t index = 0;

    for (uint32_t i = 0; i < 2048 * 6; i++) {
        indexData.push_back(index + 0);
        indexData.push_back(index + 1);
        indexData.push_back(index + 2);
        indexData.push_back(index + 1);
        indexData.push_back(index + 3);
        indexData.push_back(index + 2);
        index += 4;
    }

    m_indexCount = static_cast<uint32_t>(indexData.size());
    m_indexBufferSize = indexData.size() * sizeof(uint32_t);

    vk::BufferCreateInfo info = {};
    info.size = m_indexBufferSize;
    info.usage = vk::BufferUsageFlags::IndexBuffer | vk::BufferUsageFlags::TransferDst;
    info.sharingMode = vk::SharingMode::Exclusive;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_indexBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, info, allocInfo);
}