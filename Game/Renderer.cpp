#include "Renderer.h"
#include <memory>
#include <entt/entt.hpp>

Renderer::Renderer(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& renderGraph, VoxelEngine::CameraSystem& cameraSystem, World& world, TextureManager& textureManager) {
    m_engine = &engine;
    m_graphics = &m_engine->getGraphics();
    m_renderGraph = &renderGraph;

    m_acquireNode = &m_renderGraph->addNode<VoxelEngine::AcquireNode>(*m_engine, *m_renderGraph);
    m_presentNode = &m_renderGraph->addNode<VoxelEngine::PresentNode>(
        *m_engine, *m_renderGraph, vk::PipelineStageFlags::BottomOfPipe, *m_acquireNode
    );
    m_transferNode = &m_renderGraph->addNode<VoxelEngine::TransferNode>(*m_engine, *m_renderGraph);
    m_chunkRenderer = &m_renderGraph->addNode<ChunkRenderer>(*m_engine, *m_renderGraph, *m_acquireNode, *m_transferNode, cameraSystem, world, textureManager);
    m_mipmapGenerator = &m_renderGraph->addNode<MipmapGenerator>(*m_engine, *m_renderGraph);

    m_renderGraph->addEdge(VoxelEngine::RenderGraph::BufferEdge(m_transferNode->bufferUsage(), m_chunkRenderer->vertexBufferUsage()));
    m_renderGraph->addEdge(VoxelEngine::RenderGraph::BufferEdge(m_transferNode->bufferUsage(), m_chunkRenderer->indexBufferUsage()));
    m_renderGraph->addEdge(VoxelEngine::RenderGraph::ImageEdge(m_acquireNode->imageUsage(), m_chunkRenderer->imageUsage()));
    m_renderGraph->addEdge(VoxelEngine::RenderGraph::ImageEdge(m_transferNode->imageUsage(), m_mipmapGenerator->inputUsage()));
    m_renderGraph->addEdge(VoxelEngine::RenderGraph::ImageEdge(m_mipmapGenerator->outputUsage(), m_chunkRenderer->textureUsage()));
    m_renderGraph->addEdge(VoxelEngine::RenderGraph::ImageEdge(m_chunkRenderer->imageUsage(), m_presentNode->imageUsage()));

    m_renderGraph->bake();
}

void Renderer::wait() {
    m_renderGraph->wait();
}

void Renderer::update(VoxelEngine::Clock& clock) {
    m_renderGraph->execute();
}