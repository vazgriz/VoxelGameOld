#pragma once
#include "Engine/Engine.h"
#include "Engine/RenderGraph/AcquireNode.h"
#include <memory>

namespace VoxelEngine {
    class PresentNode : public RenderGraph::Node {
    public:
        PresentNode(VoxelEngine::Engine& engine, RenderGraph& graph, vk::PipelineStageFlags stage, AcquireNode& acquireNode);

        RenderGraph::ImageUsage& imageUsage() const { return *m_imageUsage; }

        void preRender(uint32_t currentFrame) {}
        void render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {}
        void postRender(uint32_t currentFrame);

    private:
        const vk::Queue* m_presentQueue;
        AcquireNode* m_acquireNode;
        std::unique_ptr<vk::Semaphore> m_semaphore;
        std::unique_ptr<RenderGraph::ImageUsage> m_imageUsage;
    };
}