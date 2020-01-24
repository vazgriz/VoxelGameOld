#pragma once
#include "Engine/RenderGraph/RenderGraph.h"
#include <vk_mem_alloc.h>
#include <queue>

namespace VoxelEngine {
    class TransferNode : public RenderGraph::Node {
    public:
        TransferNode(Engine& engine, RenderGraph& graph);

        void preRender(uint32_t currentFrame);
        void render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer);
        void postRender(uint32_t currentFrame) {}

        void transfer(std::shared_ptr<VoxelEngine::Buffer> buffer, vk::DeviceSize size, vk::DeviceSize offset, void* data);

    private:
        struct BufferInfo {
            const vk::Buffer* buffer;
            vk::BufferCopy copy;
        };

        struct SyncItem {
            std::shared_ptr<Buffer> buffer;
            vk::DeviceSize size;
            vk::DeviceSize offset;
        };

        VoxelEngine::Engine* m_engine;
        VoxelEngine::RenderGraph* m_renderGraph;
        std::vector<std::unique_ptr<VoxelEngine::Buffer>> m_stagingBuffers;
        std::vector<void*> m_stagingBufferPtrs;
        std::vector<BufferInfo> m_bufferCopies;
        std::queue<SyncItem> m_syncQueue;
        bool m_preRenderDone = false;
        size_t m_ptr = 0;

        void createStaging();
    };
}