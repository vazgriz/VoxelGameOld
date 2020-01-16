#include "Engine/RenderSystem.h"

using namespace VoxelEngine;

RenderSystem::RenderSystem(uint32_t priority, Graphics& renderer) : System(priority) {
    m_graphics = &renderer;

    for (size_t i = 0; i < m_graphics->swapchain().images().size(); i++) {
        vk::FenceCreateInfo info = {};
        info.flags = vk::FenceCreateFlags::Signaled;
        m_fences.emplace_back(m_graphics->device(), info);
    }

    vk::SemaphoreCreateInfo info = {};

    m_acquireSemaphore = std::make_unique<vk::Semaphore>(m_graphics->device(), info);
    m_renderFinishedSemaphore = std::make_unique<vk::Semaphore>(m_graphics->device(), info);
}

void RenderSystem::submit(const vk::CommandBuffer& commandBuffer) {
    m_commandBuffers.push_back(commandBuffer);
}

void RenderSystem::wait() const {
    vk::Fence::wait(m_graphics->device(), m_fences, true, -1);
}

void RenderSystem::preUpdate(Clock& clock) {
    m_graphics->swapchain().acquireNextImage(-1, m_acquireSemaphore.get(), nullptr, m_index);

    m_fences[m_index].wait();
    m_fences[m_index].reset();
}

void RenderSystem::update(Clock& clock) {
    vk::SubmitInfo submitInfo = {};
    submitInfo.waitSemaphores = { *m_acquireSemaphore };
    submitInfo.waitDstStageMask = { vk::PipelineStageFlags::ColorAttachmentOutput };
    submitInfo.commandBuffers = std::move(m_commandBuffers);
    submitInfo.signalSemaphores = { *m_renderFinishedSemaphore };

    m_graphics->graphicsQueue()->submit(submitInfo, &m_fences[m_index]);

    vk::PresentInfo presentInfo = {};
    presentInfo.imageIndices = { m_index };
    presentInfo.swapchains = { m_graphics->swapchain() };
    presentInfo.waitSemaphores = { *m_renderFinishedSemaphore };

    m_graphics->presentQueue()->present(presentInfo);
}