#include "MeshManager.h"

const size_t pageSize = 256 * 1024 * 1024;

MeshAllocation::MeshAllocation() {
    buffer = nullptr;
    allocation = {};
}

MeshAllocation::MeshAllocation(const std::shared_ptr<VoxelEngine::Buffer>& buffer, VoxelEngine::Allocation allocation) {
    this->buffer = buffer;
    this->allocation = allocation;
}

MeshAllocation::MeshAllocation(MeshAllocation&& other) {
    buffer = other.buffer;
    allocation = other.allocation;

    other.allocation = {};
}

MeshAllocation& MeshAllocation::operator = (MeshAllocation&& other) {
    buffer = other.buffer;
    allocation = other.allocation;

    other.allocation = {};

    return *this;
}

MeshAllocation::~MeshAllocation() {
    if (allocation.allocator != nullptr) {
        allocation.allocator->free(allocation);
    }
}

MeshManager::Page::Page(VoxelEngine::Engine& engine) : allocator(0, pageSize) {
    vk::BufferCreateInfo info = {};
    info.size = pageSize;
    info.usage = vk::BufferUsageFlags::VertexBuffer | vk::BufferUsageFlags::TransferDst;
    info.sharingMode = vk::SharingMode::Exclusive;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

    buffer = std::make_shared<VoxelEngine::Buffer>(engine, info, allocInfo);
}

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

MeshAllocation MeshManager::allocateBuffer(size_t size, size_t alignment) {
    //check existing pages
    for (auto& page : m_pages) {
        VoxelEngine::Allocation allocation = page->allocator.allocate(size, alignment);

        if (allocation.allocator != nullptr) {
            return { page->buffer, allocation };
        }
    }

    //create new page
    {
        auto& page = m_pages.emplace_back(std::make_unique<Page>(*m_engine));
        VoxelEngine::Allocation allocation = page->allocator.allocate(size, alignment);

        if (allocation.allocator != nullptr) {
            return { page->buffer, allocation };
        }
    }

    //allocation failed
    return {};
}