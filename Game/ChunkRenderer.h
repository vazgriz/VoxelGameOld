#pragma once
#include <VulkanWrapper/VulkanWrapper.h>
#include <Engine/Engine.h>
#include <Engine/RenderGraph/RenderGraph.h>
#include <Engine/RenderGraph/AcquireNode.h>
#include <Engine/RenderGraph/TransferNode.h>
#include <Engine/CameraSystem.h>
#include <entt/entt.hpp>

#include "TextureManager.h"
#include "SkyboxManager.h"
#include "SelectionBox.h"
#include "World.h"

class MeshManager;

class ChunkRenderer :public VoxelEngine::RenderGraph::Node {
public:
    ChunkRenderer(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& graph, VoxelEngine::AcquireNode& acquireNode, VoxelEngine::TransferNode& transferNode, VoxelEngine::CameraSystem& cameraSystem, World& world, TextureManager& textureManager, SkyboxManager& skyboxManager, SelectionBox& selectionBox, MeshManager& meshManager);

    vk::RenderPass& renderPass() const { return *m_renderPass; }
    VoxelEngine::RenderGraph::BufferUsage& uniformBufferUsage() const { return *m_uniformBufferUsage; }
    VoxelEngine::RenderGraph::BufferUsage& vertexBufferUsage() const { return *m_vertexBufferUsage; }
    VoxelEngine::RenderGraph::BufferUsage& indexBufferUsage() const { return *m_indexBufferUsage; }
    VoxelEngine::RenderGraph::ImageUsage& textureUsage() const { return *m_textureUsage; }
    VoxelEngine::RenderGraph::ImageUsage& skyboxUsage() const { return *m_skyboxUsage; }
    VoxelEngine::RenderGraph::ImageUsage& selectionTextureUsage() const { return *m_selectionTextureUsage; }
    VoxelEngine::RenderGraph::ImageUsage& imageUsage() const { return *m_imageUsage; }

    void preRender(uint32_t currentFrame);
    void render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer);
    void postRender(uint32_t currentFrame) {}

    VoxelEngine::Image& colorBuffer() const { return *m_colorBuffer; }
    vk::ImageView& colorBufferView() const { return *m_colorBufferView; }

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::Graphics* m_graphics;
    VoxelEngine::AcquireNode* m_acquireNode;
    VoxelEngine::TransferNode* m_transferNode;
    VoxelEngine::CameraSystem* m_cameraSystem;
    World* m_world;
    TextureManager* m_textureManager;
    SkyboxManager* m_skyboxManager;
    SelectionBox* m_selectionBox;
    MeshManager* m_meshManager;

    std::unique_ptr<VoxelEngine::Image> m_colorBuffer;
    std::unique_ptr<vk::ImageView> m_colorBufferView;
    std::unique_ptr<VoxelEngine::Image> m_depthBuffer;
    std::unique_ptr<vk::ImageView> m_depthBufferView;
    std::unique_ptr<vk::RenderPass> m_renderPass;
    std::unique_ptr<vk::Framebuffer> m_framebuffer;
    std::unique_ptr<vk::PipelineLayout> m_pipelineLayout;
    std::unique_ptr<vk::Pipeline> m_pipeline;

    std::unique_ptr<VoxelEngine::RenderGraph::BufferUsage> m_uniformBufferUsage;
    std::unique_ptr<VoxelEngine::RenderGraph::BufferUsage> m_vertexBufferUsage;
    std::unique_ptr<VoxelEngine::RenderGraph::BufferUsage> m_indexBufferUsage;
    std::unique_ptr<VoxelEngine::RenderGraph::ImageUsage> m_textureUsage;
    std::unique_ptr<VoxelEngine::RenderGraph::ImageUsage> m_skyboxUsage;
    std::unique_ptr<VoxelEngine::RenderGraph::ImageUsage> m_selectionTextureUsage;
    std::unique_ptr<VoxelEngine::RenderGraph::ImageUsage> m_imageUsage;

    void createRenderPass();
    void createColorBuffer();
    void createDepthBuffer();
    void createFramebuffers();
    void createPipelineLayout();
    void createPipeline();

    void onSwapchainChanged(vk::Swapchain& swapchain);
};