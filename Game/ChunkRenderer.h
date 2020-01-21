#pragma once
#include <VulkanWrapper/VulkanWrapper.h>
#include <Engine/Engine.h>
#include <Engine/RenderGraph/RenderGraph.h>
#include <Engine/RenderGraph/AcquireNode.h>
#include <Engine/RenderGraph/TransferNode.h>
#include <Engine/CameraSystem.h>
#include <entt/entt.hpp>

class ChunkRenderer :public VoxelEngine::RenderGraph::Node {
public:
    ChunkRenderer(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& graph, VoxelEngine::AcquireNode& acquireNode, VoxelEngine::TransferNode& transferNode, VoxelEngine::CameraSystem& cameraSystem, entt::registry& registry);

    void preRender(uint32_t currentFrame);
    void render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer);
    void postRender(uint32_t currentFrame) {}

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::Graphics* m_graphics;
    VoxelEngine::AcquireNode* m_acquireNode;
    VoxelEngine::TransferNode* m_transferNode;
    VoxelEngine::CameraSystem* m_cameraSystem;
    entt::registry* m_registry;

    std::unique_ptr<VoxelEngine::Image> m_depthBuffer;
    std::unique_ptr<vk::ImageView> m_depthBufferView;
    std::unique_ptr<vk::RenderPass> m_renderPass;
    std::vector<vk::Framebuffer> m_framebuffers;
    std::unique_ptr<vk::PipelineLayout> m_pipelineLayout;
    std::unique_ptr<vk::Pipeline> m_pipeline;

    void createDepthBuffer();
    void createRenderPass();
    void createFramebuffers();
    void createPipelineLayout();
    void createPipeline();

    void onSwapchainChanged(vk::Swapchain& swapchain);
};