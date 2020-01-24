#include "Engine/RenderGraph/TransferNode.h"
#include "Engine/Utilities.h"

using namespace VoxelEngine;

TransferNode::TransferNode(Engine& engine, RenderGraph& graph)
    : RenderGraph::Node(graph, *engine.getGraphics().transferQueue(), vk::PipelineStageFlags::TopOfPipe) {
    m_engine = &engine;
    m_renderGraph = &graph;

    createStaging();
}

void TransferNode::preRender(uint32_t currentFrame) {
    m_ptr = 0;

    while (m_syncQueue.size() > 0) {
        auto item = m_syncQueue.front();
        m_syncQueue.pop();

        sync(item.buffer, item.size, item.offset, vk::AccessFlags::TransferWrite, vk::PipelineStageFlags::Transfer);
        m_ptr = align(m_ptr + item.size, 4);
    }

    m_preRenderDone = true;
}

void TransferNode::render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    for (auto& copy : m_bufferCopies) {
        commandBuffer.copyBuffer(m_stagingBuffers[currentFrame]->buffer(), *copy.buffer, { copy.copy });
    }

    m_bufferCopies.clear();
    m_preRenderDone = false;
}

void TransferNode::createStaging() {
    vk::BufferCreateInfo info = {};
    info.size = 256 * 1024 * 1024;
    info.usage = vk::BufferUsageFlags::TransferSrc;
    info.queueFamilyIndices = { m_engine->getGraphics().transferQueue()->familyIndex() };
    info.sharingMode = vk::SharingMode::Concurrent;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    for (size_t i = 0; i < m_renderGraph->framesInFlight(); i++) {
        m_stagingBuffers.emplace_back(std::make_unique<VoxelEngine::Buffer>(*m_engine, info, allocInfo));
        m_stagingBufferPtrs.push_back(m_stagingBuffers.back()->getMapping());
    }
}

void TransferNode::transfer(std::shared_ptr<VoxelEngine::Buffer> buffer, vk::DeviceSize size, vk::DeviceSize offset, void* data) {
    if (size == 0) return;
    uint32_t currentFrame = m_renderGraph->currentFrame();
    m_ptr = align(m_ptr, 4);
    char* ptr = static_cast<char*>(m_stagingBufferPtrs[currentFrame]) + m_ptr;

    memcpy(ptr, data, size);

    vk::BufferCopy copy = {};
    copy.srcOffset = m_ptr;
    copy.dstOffset = offset;
    copy.size = size;

    m_bufferCopies.push_back({ &buffer->buffer(), copy });

    m_ptr += size;

    if (m_preRenderDone) {
        sync(buffer, size, offset, vk::AccessFlags::TransferWrite, vk::PipelineStageFlags::Transfer);
    } else {
        m_syncQueue.push({ buffer, size, offset });
    }
}