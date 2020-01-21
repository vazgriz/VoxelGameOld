#pragma once
#include <vk_mem_alloc.h>
#include <VulkanWrapper/VulkanWrapper.h>
#include "Engine/Engine.h"
#include "Engine/RenderGraph/RenderGraph.h"

namespace VoxelEngine {
    class Engine;

    struct BufferState {
        Engine* engine;
        vk::Buffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocationInfo;

        BufferState(Engine* engine, vk::Buffer&& buffer, VmaAllocation allocation, VmaAllocationInfo info);
        ~BufferState();
    };

    class Buffer {
    public:
        Buffer(Engine& engine, const vk::BufferCreateInfo& info, const VmaAllocationCreateInfo& allocInfo);

        vk::Buffer& buffer() const { return m_state->buffer; }
        void* getMapping() const;
        size_t size() const { return m_state->buffer.size(); }

        std::shared_ptr<BufferState> state() const { return m_state; }

    private:
        Engine* m_engine;
        std::shared_ptr<BufferState> m_state;
    };
}