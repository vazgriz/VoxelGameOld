#pragma once
#include <Engine/RenderGraph/RenderGraph.h>
#include <Engine/RenderGraph/AcquireNode.h>

class CompositorNode : public VoxelEngine::RenderGraph::Node {
public:
    CompositorNode(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& renderGraph, VoxelEngine::AcquireNode& acquireNode);

    CompositorNode(const CompositorNode& other) = delete;
    CompositorNode& operator = (const CompositorNode& other) = delete;
    CompositorNode(CompositorNode&& other) = default;
    CompositorNode& operator = (CompositorNode&& other) = default;

    VoxelEngine::RenderGraph::ImageUsage& mainUsage() const { return *m_mainUsage; }
    VoxelEngine::RenderGraph::ImageUsage& uiUsage() const { return *m_uiUsage; }

    void preRender(uint32_t currentFrame) {};
    void render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer);
    void postRender(uint32_t currentFrame) {};

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::Graphics* m_graphics;
    VoxelEngine::AcquireNode* m_acquireNode;

    std::unique_ptr<VoxelEngine::RenderGraph::ImageUsage> m_mainUsage;
    std::unique_ptr<VoxelEngine::RenderGraph::ImageUsage> m_uiUsage;

    std::unique_ptr<vk::RenderPass> m_renderPass;
    std::vector<vk::Framebuffer> m_framebuffers;
    std::unique_ptr<vk::PipelineLayout> m_pipelineLayout;
    std::unique_ptr<vk::Pipeline> m_pipeline;

    void createRenderPass();
    void createFramebuffers();
    void createPipelineLayout();
    void createPipeline();
};