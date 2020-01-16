#include "Engine/Buffer.h"

using namespace VoxelEngine;

Buffer::Buffer(Engine& engine, const vk::BufferCreateInfo& info, const VmaAllocationCreateInfo& allocInfo) {
    m_engine = &engine;

    VmaAllocator allocator = engine.getGraphics().memory().allocator();
    info.marshal();

   VkBuffer buffer;
   VmaAllocation allocation;
   VmaAllocationInfo allocationInfo;
   vmaCreateBuffer(allocator, info.getInfo(), &allocInfo, &buffer, &allocation, &allocationInfo);
   
   m_buffer = std::make_unique<vk::Buffer>(engine.getGraphics().device(), buffer, true, &info);
   m_allocation = allocation;
   m_allocationInfo = allocationInfo;
}

Buffer::~Buffer() {
    vmaFreeMemory(m_engine->getGraphics().memory().allocator(), m_allocation);
}

void* Buffer::getMapping() const {
    return m_allocationInfo.pMappedData;
}