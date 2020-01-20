#include "Engine/Image.h"
#include "Engine/Engine.h"

using namespace VoxelEngine;

Image::Image(Engine& engine, const vk::ImageCreateInfo& info, const VmaAllocationCreateInfo& allocInfo) {
    m_engine = &engine;

    VmaAllocator allocator = engine.getGraphics().memory().allocator();
    info.marshal();

    VkImage buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    vmaCreateImage(allocator, info.getInfo(), &allocInfo, &buffer, &allocation, &allocationInfo);

    m_image = std::make_unique<vk::Image>(engine.getGraphics().device(), buffer, true, &info);
    m_allocation = allocation;
    m_allocationInfo = allocationInfo;
}

Image::~Image() {
    vmaFreeMemory(m_engine->getGraphics().memory().allocator(), m_allocation);
}