#include "TriangleRenderer.h"
#include <iostream>

TriangleRenderer::TriangleRenderer(uint32_t priority, Renderer& renderer, RenderSystem& renderSystem) : System(priority) {
    m_renderer = &renderer;
    m_renderSystem = &renderSystem;

    vk::CommandPoolCreateInfo info = {};
    info.queueFamilyIndex = m_renderer->graphicsQueue()->familyIndex();
    info.flags = vk::CommandPoolCreateFlags::ResetCommandBuffer;

    m_commandPool = std::make_unique<vk::CommandPool>(m_renderer->device(), info);

    vk::CommandBufferAllocateInfo bufferInfo = {};
    bufferInfo.commandPool = m_commandPool.get();
    bufferInfo.commandBufferCount = static_cast<uint32_t>(m_renderer->swapchain().images().size());

    m_commandBuffers = m_commandPool->allocate(bufferInfo);

    createRenderPass();
    createFramebuffers();
}

void TriangleRenderer::createRenderPass() {
    vk::AttachmentDescription attachment = {};
    attachment.format = m_renderer->swapchain().format();
    attachment.samples = vk::SampleCountFlags::_1;
    attachment.loadOp = vk::AttachmentLoadOp::DontCare;
    attachment.storeOp = vk::AttachmentStoreOp::Store;
    attachment.stencilLoadOp = vk::AttachmentLoadOp::DontCare;
    attachment.stencilStoreOp = vk::AttachmentStoreOp::DontCare;
    attachment.initialLayout = vk::ImageLayout::Undefined;
    attachment.finalLayout = vk::ImageLayout::PresentSrcKhr;

    vk::AttachmentReference ref = {};
    ref.attachment = 0;
    ref.layout = vk::ImageLayout::ColorAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::Graphics;
    subpass.colorAttachments = { ref };

    vk::RenderPassCreateInfo info = {};
    info.attachments = { attachment };
    info.subpasses = { subpass };

    m_renderPass = std::make_unique<vk::RenderPass>(m_renderer->device(), info);
}

void TriangleRenderer::createFramebuffers() {
    for (size_t i = 0; i < m_renderer->swapchain().images().size(); i++) {
        vk::FramebufferCreateInfo info = {};
        info.renderPass = m_renderPass.get();
        info.attachments = { m_renderer->swapchainImageViews()[i] };
        info.width = m_renderer->swapchain().extent().width;
        info.height = m_renderer->swapchain().extent().height;
        info.layers = 1;

        m_framebuffers.emplace_back(m_renderer->device(), info);
    }
}

void TriangleRenderer::update(Clock& clock) {
    uint32_t index = m_renderSystem->getIndex();
    vk::CommandBuffer& commandBuffer = m_commandBuffers[index];

    commandBuffer.reset(vk::CommandBufferResetFlags::None);

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlags::OneTimeSubmit;

    commandBuffer.begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_renderPass.get();
    renderPassInfo.framebuffer = &m_framebuffers[index];
    renderPassInfo.renderArea.extent = m_renderer->swapchain().extent();
    renderPassInfo.clearValues = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::Inline);

    commandBuffer.endRenderPass();
    commandBuffer.end();

    m_renderSystem->submit(commandBuffer);
}