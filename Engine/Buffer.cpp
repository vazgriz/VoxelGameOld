#include "Engine/Buffer.h"

using namespace VoxelEngine;

BufferState::BufferState(Engine* engine, vk::Buffer&& buffer, VmaAllocation allocation) : buffer(std::move(buffer)) {
    this->engine = engine;
    this->allocation = allocation;
}

BufferState::BufferState(BufferState&& other) : buffer(std::move(other.buffer)) {
    engine = other.engine;
    allocation = other.allocation;
    other.allocation = {};
}

BufferState& BufferState::operator = (BufferState&& other) {
    engine = other.engine;
    allocation = other.allocation;
    other.allocation = VK_NULL_HANDLE;
    return *this;
}

BufferState::~BufferState() {
    vmaFreeMemory(engine->getGraphics().memory().allocator(), allocation);
}

Buffer::Buffer(Engine& engine, const vk::BufferCreateInfo& info, const VmaAllocationCreateInfo& allocInfo) {
    m_engine = &engine;

    VmaAllocator allocator = engine.getGraphics().memory().allocator();
    info.marshal();

   VkBuffer buffer;
   VmaAllocation allocation;
   vmaCreateBuffer(allocator, info.getInfo(), &allocInfo, &buffer, &allocation, &m_allocationInfo);
   
   m_bufferState = std::make_unique<BufferState>(m_engine, vk::Buffer(engine.getGraphics().device(), buffer, true, &info), allocation);
}

Buffer::~Buffer() {
    m_engine->renderGraph().queueDestroy(std::move(*m_bufferState));
}

void* Buffer::getMapping() const {
    return m_allocationInfo.pMappedData;
}