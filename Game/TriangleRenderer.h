#pragma once
#include <VulkanWrapper/VulkanWrapper.h>
#include <Engine/Engine.h>
#include <Engine/RenderGraph/RenderGraph.h>
#include <Engine/RenderGraph/AcquireNode.h>

class TriangleRenderer :public VoxelEngine::RenderGraph::Node {
public:
    TriangleRenderer(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& graph, VoxelEngine::AcquireNode& acquireNode);

    void preRender(uint32_t currentFrame) {}
    void render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer);
    void postRender(uint32_t currentFrame) {}

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::Graphics* m_graphics;
    VoxelEngine::AcquireNode* m_acquireNode;
    std::unique_ptr<vk::RenderPass> m_renderPass;
    std::vector<vk::Framebuffer> m_framebuffers;
    std::unique_ptr<vk::PipelineLayout> m_pipelineLayout;
    std::unique_ptr<vk::Pipeline> m_pipeline;
    std::unique_ptr<VoxelEngine::Mesh> m_mesh;

    void createRenderPass();
    void createFramebuffers();
    void createPipelineLayout();
    void createPipeline();
    void createMesh();
};