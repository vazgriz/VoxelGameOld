#include "Engine/RenderSystem.h"

RenderSystem::RenderSystem(uint32_t priority, Renderer& renderer) : System(priority) {
    m_renderer = &renderer;

    for (size_t i = 0; i < m_renderer->swapchain().images().size(); i++) {
        vk::FenceCreateInfo info = {};
        info.flags = vk::FenceCreateFlags::Signaled;
        m_fences.emplace_back(m_renderer->device(), info);
    }

    vk::SemaphoreCreateInfo info = {};

    m_acquireSemaphore = std::make_unique<vk::Semaphore>(m_renderer->device(), info);
    m_renderFinishedSemaphore = std::make_unique<vk::Semaphore>(m_renderer->device(), info);
}

void RenderSystem::submit(const vk::CommandBuffer& commandBuffer) {
    m_commandBuffers.push_back(commandBuffer);
}

void RenderSystem::wait() const {
    vk::Fence::wait(m_renderer->device(), m_fences, true, -1);
}

void RenderSystem::preUpdate(Clock& clock) {
    m_renderer->swapchain().acquireNextImage(-1, m_acquireSemaphore.get(), nullptr, m_index);

    m_fences[m_index].wait();
    m_fences[m_index].reset();
}

void RenderSystem::update(Clock& clock) {
    vk::SubmitInfo submitInfo = {};
    submitInfo.waitSemaphores = { *m_acquireSemaphore };
    submitInfo.waitDstStageMask = { vk::PipelineStageFlags::ColorAttachmentOutput };
    submitInfo.commandBuffers = std::move(m_commandBuffers);
    submitInfo.signalSemaphores = { *m_renderFinishedSemaphore };

    m_renderer->graphicsQueue()->submit(submitInfo, &m_fences[m_index]);

    vk::PresentInfo presentInfo = {};
    presentInfo.imageIndices = { m_index };
    presentInfo.swapchains = { m_renderer->swapchain() };
    presentInfo.waitSemaphores = { *m_renderFinishedSemaphore };

    m_renderer->presentQueue()->present(presentInfo);
}