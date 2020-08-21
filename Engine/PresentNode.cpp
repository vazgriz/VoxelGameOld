#include "Engine/RenderGraph/PresentNode.h"

using namespace VoxelEngine;

PresentNode::PresentNode(VoxelEngine::Engine& engine, RenderGraph& graph, vk::PipelineStageFlags stage, AcquireNode& acquireNode)
    : RenderGraph::Node(graph, *engine.getGraphics().presentQueue(), stage) {
    m_presentQueue = engine.getGraphics().presentQueue();
    m_acquireNode = &acquireNode;

    vk::SemaphoreCreateInfo info = {};
    m_semaphore = std::make_unique<vk::Semaphore>(m_presentQueue->device(), info);

    //Dummy ImageUsage. Used to set up semaphore. This should not have any images submitted to it.
    m_imageUsage = std::make_unique<RenderGraph::ImageUsage>(*this, vk::ImageLayout::Undefined, vk::AccessFlags::None, vk::PipelineStageFlags::BottomOfPipe);

    addExternalSignal(*m_semaphore);
}

void PresentNode::postRender(uint32_t currentFrame) {
    vk::PresentInfo info = {};
    info.swapchains = { m_acquireNode->swapchain() };
    info.imageIndices = { m_acquireNode->swapchainIndex() };
    info.waitSemaphores = { *m_semaphore };

    m_presentQueue->present(info);
}