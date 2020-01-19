#pragma once
#include <vk_mem_alloc.h>
#include <VulkanWrapper/VulkanWrapper.h>
#include "Engine/Engine.h"

namespace VoxelEngine {
    class Engine;

    class Buffer {
    public:
        Buffer(Engine& engine, const vk::BufferCreateInfo& info, const VmaAllocationCreateInfo& allocInfo);
        ~Buffer();

        vk::Buffer& buffer() const { return *m_buffer; }
        void* getMapping() const;
        size_t size() const { return m_buffer->size(); }

    private:
        Engine* m_engine;
        std::unique_ptr<vk::Buffer> m_buffer;
        VmaAllocation m_allocation;
        VmaAllocationInfo m_allocationInfo;
    };
}