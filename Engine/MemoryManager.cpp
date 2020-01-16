#include "Engine/MemoryManager.h"

using namespace VoxelEngine;

MemoryManager::MemoryManager(vk::Device& device) {
    createAllocator(device);
}

MemoryManager::~MemoryManager() {
    vmaDestroyAllocator(m_allocator);
}

void MemoryManager::createAllocator(vk::Device& device) {
    VmaAllocatorCreateInfo info = {};
    info.physicalDevice = device.physicalDevice().handle();
    info.device = device.handle();

    vmaCreateAllocator(&info, &m_allocator);
}