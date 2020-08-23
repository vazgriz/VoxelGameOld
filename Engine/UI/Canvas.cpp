#include "Engine/UI/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/UI/Transform.h"
#include "Engine/UI/Element.h"
#include "Engine/RenderGraph/TransferNode.h"
#include "Engine/Buffer.h"
#include "glm/gtc/matrix_transform.hpp"

using namespace VoxelEngine;
using namespace VoxelEngine::UI;

struct UniformData {
    glm::mat4 projection;
};

Canvas::Canvas(Engine& engine, TransferNode& transferNode, uint32_t width, uint32_t height) {
    m_engine = &engine;
    m_graphics = &engine.getGraphics();
    m_transferNode = &transferNode;
    m_renderPass = nullptr;
    m_descriptorSetLayout = nullptr;

    createUniformBuffer();
    createDescriptorPool();

    setSize(width, height);
}

void Canvas::preRender() {
    UniformData uniform = {};
    uniform.projection = glm::orthoRH_ZO(0.0f, static_cast<float>(m_width), 0.0f, static_cast<float>(m_height), -1.0f, 1.0f);

    m_transferNode->transfer(*m_uniformBuffer, sizeof(UniformData), 0, &uniform);
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

void Canvas::setCameraDescriptorLayout(vk::DescriptorSetLayout& layout) {
    m_descriptorSetLayout = &layout;

    updateResources();
}

void Canvas::updateResources() {
    if (m_renderPass != nullptr) {
        createImage();
        createImageView();
        createFramebuffer();
    }

    if (m_descriptorSetLayout != nullptr) {
        createDescriptorSet();
        writeDescriptor();
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

void Canvas::createUniformBuffer() {
    vk::BufferCreateInfo info = {};
    info.size = sizeof(UniformData);
    info.usage = vk::BufferUsageFlags::UniformBuffer | vk::BufferUsageFlags::TransferDst;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_uniformBuffer = std::make_unique<Buffer>(*m_engine, info, allocInfo);
}

void Canvas::createDescriptorPool() {
    vk::DescriptorPoolCreateInfo info = {};
    info.maxSets = 1;
    info.poolSizes = { { vk::DescriptorType::UniformBuffer, 1 } };

    m_descriptorPool = std::make_unique<vk::DescriptorPool>(m_engine->getGraphics().device(), info);
}

void Canvas::createDescriptorSet() {
    vk::DescriptorSetAllocateInfo info = {};
    info.descriptorPool = m_descriptorPool.get();
    info.setLayouts = { *m_descriptorSetLayout };

    m_descriptorSet = std::make_unique<vk::DescriptorSet>(std::move(m_descriptorPool->allocate(info)[0]));
}

void Canvas::writeDescriptor() {
    vk::DescriptorBufferInfo info = {};
    info.buffer = &m_uniformBuffer->buffer();
    info.range = sizeof(UniformData);

    vk::WriteDescriptorSet write = {};
    write.dstSet = m_descriptorSet.get();
    write.bufferInfo = { info };
    write.descriptorType = vk::DescriptorType::UniformBuffer;

    vk::DescriptorSet::update(m_engine->getGraphics().device(), { write }, nullptr);
}