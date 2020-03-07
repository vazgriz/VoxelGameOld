#pragma once
#include <VulkanWrapper/VulkanWrapper.h>
#include <Engine/Engine.h>
#include <Engine/RenderGraph/TransferNode.h>

struct MeshAllocation {
    std::shared_ptr<VoxelEngine::Buffer> buffer;
    VoxelEngine::Allocation allocation;

    MeshAllocation();
    MeshAllocation(const std::shared_ptr<VoxelEngine::Buffer>& buffer, VoxelEngine::Allocation allocation);
    MeshAllocation(const MeshAllocation& other) = delete;
    MeshAllocation& operator = (const MeshAllocation& other) = delete;
    MeshAllocation(MeshAllocation&& other);
    MeshAllocation& operator = (MeshAllocation&& other);
    ~MeshAllocation();
};

class MeshManager {
    struct Page {
        VoxelEngine::FreeListAllocator allocator;
        std::shared_ptr<VoxelEngine::Buffer> buffer;

        Page(VoxelEngine::Engine& engine);
    };

public:
    MeshManager(VoxelEngine::Engine& engine);

    std::shared_ptr<VoxelEngine::Buffer>& indexBuffer() { return m_indexBuffer; }

    void setTransferNode(VoxelEngine::TransferNode& transferNode);

    MeshAllocation allocateBuffer(size_t size, size_t alignment);

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::TransferNode* m_transferNode;

    std::vector<std::unique_ptr<Page>> m_pages;

    std::shared_ptr<VoxelEngine::Buffer> m_indexBuffer;
    uint32_t m_indexCount;
    size_t m_indexBufferSize;

    void createIndexBuffer(std::vector<uint32_t>& indexData);
};