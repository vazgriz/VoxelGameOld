#pragma once
#include "Engine/Engine.h"
#include <VulkanWrapper/VulkanWrapper.h>
#include <memory>

namespace VoxelEngine {
    class Buffer;

    class Mesh {
    public:
        Mesh();

        size_t indexCount() const { return m_indexCount; }
        size_t vertexCount() const { return m_vertexCount; }
        void setIndexCount(uint32_t indexCount) { m_indexCount = indexCount; }
        void setVertexCount(uint32_t vertexCount) { m_vertexCount = vertexCount; }

        const std::vector<std::shared_ptr<VoxelEngine::Buffer>>& bindings() const { return m_bindings; }
        const std::shared_ptr<Buffer>& indexBuffer() const { return m_indexBuffer; }

        void addBinding(std::shared_ptr<Buffer> buffer, vk::DeviceSize offset = 0);
        Buffer& getBinding(size_t index) const { return *m_bindings[index]; }
        size_t bindingCount() const { return m_bindings.size(); }

        void setIndexBuffer(std::shared_ptr<Buffer> buffer, vk::IndexType type, vk::DeviceSize offset);
        bool hasIndexBuffer() const { return m_indexBuffer != nullptr; }

        void clearBindings();
        void clearIndexBuffer();

        void draw(vk::CommandBuffer& commandBuffer, uint32_t vertexCount) const;
        void drawIndexed(vk::CommandBuffer& commandBuffer, uint32_t indexCount) const;
        void draw(vk::CommandBuffer& commandBuffer) const;
        void drawIndexed(vk::CommandBuffer& commandBuffer) const;

    private:
        std::vector<std::shared_ptr<VoxelEngine::Buffer>> m_bindings;
        std::shared_ptr<Buffer> m_indexBuffer;
        vk::IndexType m_indexType;
        vk::DeviceSize m_indexOffset;
        uint32_t m_indexCount;

        std::vector<std::reference_wrapper<const vk::Buffer>> m_buffers;
        std::vector<vk::DeviceSize> m_offsets;
        uint32_t m_vertexCount;
    };
}