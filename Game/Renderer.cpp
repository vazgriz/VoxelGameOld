#include "Renderer.h"
#include <memory>

Renderer::Renderer(uint32_t priority, VoxelEngine::Engine& engine) : VoxelEngine::System(priority) {
    m_engine = &engine;
    m_graphics = &m_engine->getGraphics();
    m_renderGraph = std::make_unique<VoxelEngine::RenderGraph>(engine.getGraphics().device(), 2);

    m_acquireNode = &m_renderGraph->addNode<VoxelEngine::AcquireNode>(*m_engine, *m_renderGraph);
    m_presentNode = &m_renderGraph->addNode<VoxelEngine::PresentNode>(
        *m_engine, *m_renderGraph, vk::PipelineStageFlags::BottomOfPipe, *m_acquireNode
    );
    m_transferNode = &m_renderGraph->addNode<VoxelEngine::TransferNode>(*m_engine, *m_renderGraph);
    m_triangleRenderer = &m_renderGraph->addNode<TriangleRenderer>(*m_engine, *m_renderGraph, *m_acquireNode, *m_transferNode);

    m_renderGraph->addEdge({ *m_transferNode, *m_triangleRenderer });
    m_renderGraph->addEdge({ *m_acquireNode, *m_triangleRenderer });
    m_renderGraph->addEdge({ *m_triangleRenderer, *m_presentNode });

    m_renderGraph->bake();
}

void Renderer::wait() {
    m_renderGraph->wait();
}

void Renderer::update(VoxelEngine::Clock& clock) {
    m_renderGraph->execute();
}