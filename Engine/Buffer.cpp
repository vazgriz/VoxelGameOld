#include "Engine/Buffer.h"

using namespace VoxelEngine;

BufferState::BufferState(Engine* engine, vk::Buffer&& buffer, VmaAllocation allocation, VmaAllocationInfo info) : buffer(std::move(buffer)) {
    this->engine = engine;
    this->allocation = allocation;
    this->allocationInfo = info;
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
   VmaAllocationInfo allocationInfo;
   vmaCreateBuffer(allocator, info.getInfo(), &allocInfo, &buffer, &allocation, &allocationInfo);
   
   m_state = std::make_shared<BufferState>(m_engine, vk::Buffer(engine.getGraphics().device(), buffer, true, &info), allocation, allocationInfo);
}

void* Buffer::getMapping() const {
    return m_state->allocationInfo.pMappedData;
}