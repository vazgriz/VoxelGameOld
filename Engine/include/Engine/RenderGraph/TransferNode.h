#pragma once
#include "Engine/RenderGraph/RenderGraph.h"
#include <vk_mem_alloc.h>
#include <queue>

namespace VoxelEngine {
    class TransferNode : public RenderGraph::Node {
    public:
        TransferNode(Engine& engine, RenderGraph& graph);

        RenderGraph::BufferUsage& bufferUsage() const { return *m_bufferUsage; }
        RenderGraph::ImageUsage& imageUsage() const { return *m_imageUsage; }

        void preRender(uint32_t currentFrame);
        void render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer);
        void postRender(uint32_t currentFrame) {}

        void transfer(Buffer& buffer, vk::DeviceSize size, vk::DeviceSize offset, void* data);
        void transfer(Image& image, vk::Offset3D offset, vk::Extent3D extent, vk::ImageSubresourceLayers subresourceLayers, void* data);

    private:
        struct BufferInfo {
            const vk::Buffer* buffer;
            vk::BufferCopy copy;
        };

        struct SyncBuffer {
            Buffer* buffer;
            vk::DeviceSize size;
            vk::DeviceSize offset;
        };

        struct ImageInfo {
            const vk::Image* image;
            vk::BufferImageCopy copy;
        };

        struct SyncImage {
            Image* image;
            vk::DeviceSize size;
            vk::ImageSubresourceLayers subresourceLayers;
        };

        VoxelEngine::Engine* m_engine;
        VoxelEngine::RenderGraph* m_renderGraph;
        std::unique_ptr<RenderGraph::BufferUsage> m_bufferUsage;
        std::unique_ptr<RenderGraph::ImageUsage> m_imageUsage;
        std::vector<std::unique_ptr<VoxelEngine::Buffer>> m_stagingBuffers;
        std::vector<void*> m_stagingBufferPtrs;
        std::vector<BufferInfo> m_bufferCopies;
        std::queue<SyncBuffer> m_syncBufferQueue;
        std::vector<ImageInfo> m_imageCopies;
        std::queue<SyncImage> m_syncImageQueue;
        bool m_preRenderDone = false;
        size_t m_ptr = 0;

        void createStaging();
    };
}