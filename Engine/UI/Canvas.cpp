#include "Engine/UI/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/UI/Transform.h"
#include "Engine/UI/Element.h"

using namespace VoxelEngine;
using namespace VoxelEngine::UI;

Canvas::Canvas(Engine& engine, uint32_t width, uint32_t height) {
    m_engine = &engine;
    m_graphics = &engine.getGraphics();
    m_renderPass = nullptr;

    setSize(width, height);
}

void Canvas::render(vk::CommandBuffer& commandBuffer) {
    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffer.get();
    renderPassInfo.renderArea.extent = m_graphics->swapchain().extent();
    renderPassInfo.clearValues.push_back({ 0.0f, 0.0f, 0.0f, 0.0f });

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::Inline);

    auto view = m_registry.view<Transform, ElementUPtr>();

    for (auto entity : view) {
        auto& transform = view.get<Transform>(entity);
        auto& element = view.get<ElementUPtr>(entity);

        element->render(*this, commandBuffer);
    }

    commandBuffer.endRenderPass();
}

void Canvas::setSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;

    updateResources();
}

void Canvas::setRenderPass(vk::RenderPass& renderPass) {
    m_renderPass = &renderPass;

    updateResources();
}

void Canvas::updateResources() {
    if (m_renderPass != nullptr) {
        createImage();
        createImageView();
        createFramebuffer();
    }
}

void Canvas::addRoot(entt::entity entity) {
    m_roots.push_back(entity);
}

entt::entity Canvas::createRoot() {
    entt::entity result = createNode();
    addRoot(result);
    return result;
}

entt::entity Canvas::createNode() {
    entt::entity entity = m_registry.create();
    return entity;
}

void Canvas::createImage() {
    vk::ImageCreateInfo info = {};
    info.extent.width = m_width;
    info.extent.height = m_height;
    info.extent.depth = 1;
    info.format = vk::Format::R8G8B8A8_Unorm;
    info.usage = vk::ImageUsageFlags::Sampled | vk::ImageUsageFlags::ColorAttachment;
    info.initialLayout = vk::ImageLayout::Undefined;
    info.imageType = vk::ImageType::_2D;
    info.samples = vk::SampleCountFlags::_1;
    info.arrayLayers = 1;
    info.mipLevels = 1;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_image = std::make_unique<VoxelEngine::Image>(*m_engine, info, allocInfo);
}

void Canvas::createImageView() {
    vk::ImageViewCreateInfo info = {};
    info.image = &m_image->image();
    info.format = vk::Format::R8G8B8A8_Unorm;
    info.viewType = vk::ImageViewType::_2D;
    info.subresourceRange.aspectMask = vk::ImageAspectFlags::Color;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.levelCount = 1;

    m_imageView = std::make_unique<vk::ImageView>(m_graphics->device(), info);
}

void Canvas::createFramebuffer() {
    vk::FramebufferCreateInfo info = {};
    info.renderPass = m_renderPass;
    info.attachments = { *m_imageView };
    info.width = m_width;
    info.height = m_height;
    info.layers = 1;

    m_framebuffer = std::make_unique<vk::Framebuffer>(m_graphics->device(), info);
}