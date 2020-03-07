#pragma once
#include <VulkanWrapper/VulkanWrapper.h>
#include <Engine/Engine.h>
#include <Engine/RenderGraph/TransferNode.h>

class MeshManager {
public:
    MeshManager(VoxelEngine::Engine& engine);

    std::shared_ptr<VoxelEngine::Buffer>& indexBuffer() { return m_indexBuffer; }

    void setTransferNode(VoxelEngine::TransferNode& transferNode);

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::TransferNode* m_transferNode;

    std::shared_ptr<VoxelEngine::Buffer> m_indexBuffer;
    uint32_t m_indexCount;
    size_t m_indexBufferSize;

    void createIndexBuffer(std::vector<uint32_t>& indexData);
};