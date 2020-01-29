#include "Engine/Image.h"
#include "Engine/Engine.h"

using namespace VoxelEngine;

ImageState::ImageState(Engine* engine, vk::Image&& image, VmaAllocation allocation) : image(std::move(image)) {
    this->engine = engine;
    this->allocation = allocation;
}

ImageState::ImageState(ImageState&& other) : image(std::move(other.image)) {
    engine = other.engine;
    allocation = other.allocation;
    other.allocation = {};
}

ImageState& ImageState::operator = (ImageState&& other) {
    engine = other.engine;
    allocation = other.allocation;
    other.allocation = VK_NULL_HANDLE;
    return *this;
}

ImageState::~ImageState() {
    vmaFreeMemory(engine->getGraphics().memory().allocator(), allocation);
}

Image::Image(Engine& engine, const vk::ImageCreateInfo& info, const VmaAllocationCreateInfo& allocInfo) {
    m_engine = &engine;

    VmaAllocator allocator = engine.getGraphics().memory().allocator();
    info.marshal();

    VkImage buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    vmaCreateImage(allocator, info.getInfo(), &allocInfo, &buffer, &allocation, &allocationInfo);

    m_imageState = std::make_unique<ImageState>(m_engine, vk::Image(engine.getGraphics().device(), buffer, true, &info), allocation);
    m_allocationInfo = allocationInfo;
}

Image::~Image() {
    m_engine->renderGraph().queueDestroy(std::move(*m_imageState));
}