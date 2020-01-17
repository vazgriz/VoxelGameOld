#include "Engine/RenderGraph/AcquireNode.h"

using namespace VoxelEngine;

AcquireNode::AcquireNode(RenderGraph& graph, const vk::Queue& queue, vk::Swapchain& swapchain) : RenderGraph::Node(graph, queue, vk::PipelineStageFlags::TopOfPipe) {
    m_swapchain = &swapchain;

    vk::SemaphoreCreateInfo info = {};
    m_semaphore = std::make_unique<vk::Semaphore>(swapchain.device(), info);

    addExternalWait(*m_semaphore, vk::PipelineStageFlags::ColorAttachmentOutput);
}

void AcquireNode::preRender(uint32_t currentFrame) {
    m_swapchain->acquireNextImage(-1, m_semaphore.get(), nullptr, m_swapchainIndex);
}