#include "Engine/UI/Canvas.h"
#include "Engine/Engine.h"

using namespace VoxelEngine;
using namespace VoxelEngine::UI;

Canvas::Canvas(Engine& engine, uint32_t width, uint32_t height) {
    m_engine = &engine;
    m_device = &engine.getGraphics().device();

    setSize(width, height);
}

void Canvas::setSize(uint32_t width, uint32_t height) {
    m_width = width;
    m_height = height;

    createImage();
    createImageView();
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

    m_imageView = std::make_unique<vk::ImageView>(*m_device, info);
}