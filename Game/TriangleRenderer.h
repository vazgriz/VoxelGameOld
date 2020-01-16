#pragma once
#include <VulkanWrapper/VulkanWrapper.h>
#include <Engine/Engine.h>

class TriangleRenderer :public VoxelEngine::System {
public:
    TriangleRenderer(uint32_t priority, VoxelEngine::Engine& engine, VoxelEngine::RenderSystem& renderSystem);

    void update(VoxelEngine::Clock& clock);

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::Graphics* m_graphics;
    VoxelEngine::RenderSystem* m_renderSystem;
    std::unique_ptr<vk::CommandPool> m_commandPool;
    std::vector<vk::CommandBuffer> m_commandBuffers;
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
    void transferMesh(const std::shared_ptr<VoxelEngine::Buffer>& vertexBuffer, const std::shared_ptr<VoxelEngine::Buffer>& colorBuffer, const std::shared_ptr<VoxelEngine::Buffer>& indexBuffer);
};