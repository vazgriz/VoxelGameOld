#include "Renderer.h"

Renderer::Renderer(uint32_t priority, VoxelEngine::Engine& engine) : VoxelEngine::System(priority) {
    m_engine = &engine;
    m_graphics = &m_engine->getGraphics();
    m_renderGraph = std::make_unique<VoxelEngine::RenderGraph>(engine.getGraphics().device(), 2);

    m_acquireNode = &m_renderGraph->addNode<VoxelEngine::AcquireNode>(*m_renderGraph, *m_graphics->graphicsQueue(), m_graphics->swapchain());
    m_presentNode = &m_renderGraph->addNode<VoxelEngine::PresentNode>(
        *m_renderGraph, *m_graphics->graphicsQueue(), *m_graphics->presentQueue(), vk::PipelineStageFlags::BottomOfPipe, *m_acquireNode
    );
    m_triangleRenderer = &m_renderGraph->addNode<TriangleRenderer>(*m_engine, *m_renderGraph, *m_acquireNode);

    m_renderGraph->bake();
}

void Renderer::wait() {
    m_renderGraph->wait();
}

void Renderer::update(VoxelEngine::Clock& clock) {
    m_renderGraph->execute();
}