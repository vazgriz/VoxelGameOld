#include "Engine/RenderGraph/PresentNode.h"

using namespace VoxelEngine;

PresentNode::PresentNode(RenderGraph& graph, const vk::Queue& queue, const vk::Queue& presentQueue, vk::PipelineStageFlags stage, AcquireNode& acquireNode) : RenderGraph::Node(graph, queue, stage) {
    m_presentQueue = &presentQueue;
    m_acquireNode = &acquireNode;

    vk::SemaphoreCreateInfo info = {};
    m_semaphore = std::make_unique<vk::Semaphore>(presentQueue.device(), info);

    addExternalSignal(*m_semaphore);
}

void PresentNode::postRender(uint32_t currentFrame) {
    vk::PresentInfo info = {};
    info.swapchains = { m_acquireNode->swapchain() };
    info.imageIndices = { m_acquireNode->swapchainIndex() };
    info.waitSemaphores = { *m_semaphore };

    m_presentQueue->present(info);
}