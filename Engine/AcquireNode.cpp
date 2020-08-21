#include "Engine/RenderGraph/AcquireNode.h"

using namespace VoxelEngine;

AcquireNode::AcquireNode(VoxelEngine::Engine& engine, RenderGraph& graph)
    : RenderGraph::Node(graph, *engine.getGraphics().graphicsQueue())
{
    m_swapchain = &engine.getGraphics().swapchain();

    vk::SemaphoreCreateInfo info = {};
    m_semaphore = std::make_unique<vk::Semaphore>(m_swapchain->device(), info);

    addExternalWait(*m_semaphore, vk::PipelineStageFlags::ColorAttachmentOutput);

    m_imageUsage = std::make_unique<RenderGraph::ImageUsage>(*this, vk::ImageLayout::Undefined, vk::AccessFlags::None, vk::PipelineStageFlags::BottomOfPipe);

    engine.getGraphics().onSwapchainChanged().connect<&AcquireNode::onSwapchainChanged>(this);
}

void AcquireNode::preRender(uint32_t currentFrame) {
    m_swapchain->acquireNextImage(-1, m_semaphore.get(), nullptr, m_swapchainIndex);
}

void AcquireNode::onSwapchainChanged(vk::Swapchain& swapchain) {
    m_swapchain = &swapchain;
}