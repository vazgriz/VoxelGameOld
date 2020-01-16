#pragma once
#include "Engine/Engine.h"
#include <VulkanWrapper/VulkanWrapper.h>
#include <memory>

namespace VoxelEngine {
    class Buffer;

    class Mesh {
        struct VertexBinding {
            std::shared_ptr<Buffer> buffer;
            vk::Format format;
        };

    public:
        Mesh();

        const std::vector<VertexBinding>& bindings() const { return m_bindings; }
        const std::shared_ptr<Buffer>& indexBuffer() const { return m_indexBuffer; }

        void addBinding(uint32_t vertexCount, std::shared_ptr<Buffer> buffer, vk::Format format, vk::DeviceSize offset = 0);
        void setIndexBuffer(uint32_t indexCount, std::shared_ptr<Buffer> buffer, vk::IndexType type, vk::DeviceSize offset);

        void draw(vk::CommandBuffer& commandBuffer) const;

    private:
        std::vector<VertexBinding> m_bindings;
        bool m_hasIndex = false;
        std::shared_ptr<Buffer> m_indexBuffer;
        vk::IndexType m_indexType;
        vk::DeviceSize m_indexOffset;
        uint32_t m_indexCount;

        std::vector<std::reference_wrapper<const vk::Buffer>> m_buffers;
        std::vector<vk::DeviceSize> m_offsets;
        uint32_t m_vertexCount;
    };
}