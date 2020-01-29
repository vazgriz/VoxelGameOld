#include "Engine/RenderGraph/TransferNode.h"
#include "Engine/Utilities.h"

using namespace VoxelEngine;

TransferNode::TransferNode(Engine& engine, RenderGraph& graph)
    : RenderGraph::Node(graph, *engine.getGraphics().transferQueue(), vk::PipelineStageFlags::TopOfPipe) {
    m_engine = &engine;
    m_renderGraph = &graph;

    m_bufferUsage = std::make_unique<RenderGraph::BufferUsage>(*this, vk::AccessFlags::TransferWrite, vk::PipelineStageFlags::Transfer);
    m_imageUsage = std::make_unique<RenderGraph::ImageUsage>(*this, vk::ImageLayout::TransferDstOptimal, vk::AccessFlags::TransferWrite, vk::PipelineStageFlags::Transfer);

    createStaging();
}

void TransferNode::preRender(uint32_t currentFrame) {
    m_ptr = 0;

    while (m_syncBufferQueue.size() > 0) {
        auto& item = m_syncBufferQueue.front();

        m_bufferUsage->sync(*item.buffer, item.size, item.offset);
        m_ptr = align(m_ptr + item.size, 4);

        m_syncBufferQueue.pop();
    }

    while (m_syncImageQueue.size() > 0) {
        auto& item = m_syncImageQueue.front();

        vk::ImageSubresourceRange subresource = {};
        subresource.aspectMask = item.subresourceLayers.aspectMask;
        subresource.baseArrayLayer = item.subresourceLayers.baseArrayLayer;
        subresource.layerCount = item.subresourceLayers.layerCount;
        subresource.baseMipLevel = item.subresourceLayers.mipLevel;
        subresource.levelCount = 1;

        m_imageUsage->sync(*item.image, subresource);
        m_ptr = align(m_ptr + item.size, 4);

        m_syncImageQueue.pop();
    }

    m_preRenderDone = true;
}

void TransferNode::render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    for (auto& copy : m_bufferCopies) {
        commandBuffer.copyBuffer(m_stagingBuffers[currentFrame]->buffer(), *copy.buffer, { copy.copy });
    }

    for (auto& copy : m_imageCopies) {
        vk::ImageMemoryBarrier barrier = {};
        barrier.image = copy.image;
        barrier.oldLayout = vk::ImageLayout::Undefined;
        barrier.newLayout = vk::ImageLayout::TransferDstOptimal;
        barrier.srcAccessMask = vk::AccessFlags::None;
        barrier.dstAccessMask = vk::AccessFlags::TransferWrite;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = copy.copy.imageSubresource.aspectMask;
        barrier.subresourceRange.baseArrayLayer = copy.copy.imageSubresource.baseArrayLayer;
        barrier.subresourceRange.layerCount = copy.copy.imageSubresource.layerCount;
        barrier.subresourceRange.baseMipLevel = copy.copy.imageSubresource.mipLevel;
        barrier.subresourceRange.levelCount = 1;

        commandBuffer.pipelineBarrier(vk::PipelineStageFlags::TopOfPipe, vk::PipelineStageFlags::Transfer, {},
            nullptr,
            nullptr,
            { barrier }
        );

        commandBuffer.copyBufferToImage(m_stagingBuffers[currentFrame]->buffer(), *copy.image, vk::ImageLayout::TransferDstOptimal, { copy.copy });
    }

    m_bufferCopies.clear();
    m_imageCopies.clear();
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

void TransferNode::transfer(Buffer& buffer, vk::DeviceSize size, vk::DeviceSize offset, void* data) {
    if (size == 0) return;
    uint32_t currentFrame = m_renderGraph->currentFrame();
    m_ptr = align(m_ptr, 4);
    char* ptr = static_cast<char*>(m_stagingBufferPtrs[currentFrame]) + m_ptr;

    memcpy(ptr, data, size);

    vk::BufferCopy copy = {};
    copy.srcOffset = m_ptr;
    copy.dstOffset = offset;
    copy.size = size;

    m_bufferCopies.push_back({ &buffer.buffer(), copy });

    m_ptr += size;

    if (m_preRenderDone) {
        m_bufferUsage->sync(buffer, size, offset);
    } else {
        m_syncBufferQueue.push({ &buffer, size, offset });
    }
}

void TransferNode::transfer(Image& image, vk::Offset3D offset, vk::Extent3D extent, vk::ImageSubresourceLayers subresourceLayers, void* data) {
    size_t size = extent.width * extent.height * extent.depth * vk::getFormatSize(image.image().format());
    if (size == 0) return;
    uint32_t currentFrame = m_renderGraph->currentFrame();
    m_ptr = align(m_ptr, 4);
    char* ptr = static_cast<char*>(m_stagingBufferPtrs[currentFrame]) + m_ptr;

    memcpy(ptr, data, size);

    vk::BufferImageCopy copy = {};
    copy.bufferOffset = m_ptr;
    copy.imageOffset = offset;
    copy.imageExtent = extent;
    copy.imageSubresource = subresourceLayers;

    m_imageCopies.push_back({ &image.image(), copy });

    m_ptr += size;

    if (m_preRenderDone) {
        vk::ImageSubresourceRange subresource = {};
        subresource.aspectMask = subresourceLayers.aspectMask;
        subresource.baseArrayLayer = subresourceLayers.baseArrayLayer;
        subresource.layerCount = subresourceLayers.layerCount;
        subresource.baseMipLevel = subresourceLayers.mipLevel;
        subresource.levelCount = 1;

        m_imageUsage->sync(image, subresource);
    } else {
        m_syncImageQueue.push({ &image, size, subresourceLayers });
    }
}