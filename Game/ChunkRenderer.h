#pragma once
#include <VulkanWrapper/VulkanWrapper.h>
#include <Engine/Engine.h>
#include <Engine/RenderGraph/RenderGraph.h>
#include <Engine/RenderGraph/AcquireNode.h>
#include <Engine/RenderGraph/TransferNode.h>
#include <Engine/CameraSystem.h>
#include "Chunk.h"

class ChunkRenderer :public VoxelEngine::RenderGraph::Node {
public:
    ChunkRenderer(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& graph, VoxelEngine::AcquireNode& acquireNode, VoxelEngine::TransferNode& transferNode, VoxelEngine::CameraSystem& cameraSystem, Chunk& chunk);

    void preRender(uint32_t currentFrame);
    void render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer);
    void postRender(uint32_t currentFrame) {}

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::Graphics* m_graphics;
    VoxelEngine::AcquireNode* m_acquireNode;
    VoxelEngine::TransferNode* m_transferNode;
    VoxelEngine::CameraSystem* m_cameraSystem;
    Chunk* m_chunk;

    std::unique_ptr<VoxelEngine::Image> m_depthBuffer;
    std::unique_ptr<vk::ImageView> m_depthBufferView;
    std::unique_ptr<vk::RenderPass> m_renderPass;
    std::vector<vk::Framebuffer> m_framebuffers;
    std::unique_ptr<vk::PipelineLayout> m_pipelineLayout;
    std::unique_ptr<vk::Pipeline> m_pipeline;
    std::unique_ptr<VoxelEngine::Mesh> m_mesh;
    std::shared_ptr<VoxelEngine::Buffer> m_vertices;
    std::shared_ptr<VoxelEngine::Buffer> m_colors;
    std::shared_ptr<VoxelEngine::Buffer> m_indices;
    std::vector<glm::ivec3> m_vertexData;
    std::vector<glm::i8vec4> m_colorData;
    std::vector<uint32_t> m_indexData;

    void createDepthBuffer();
    void createRenderPass();
    void createFramebuffers();
    void createPipelineLayout();
    void createPipeline();
    void createMesh();
    void transferMesh();

    void onSwapchainChanged(vk::Swapchain& swapchain);
};