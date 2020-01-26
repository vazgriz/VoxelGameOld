#include "MipmapGenerator.h"

MipmapGenerator::MipmapGenerator(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& graph)
    : VoxelEngine::RenderGraph::Node(graph, *engine.getGraphics().graphicsQueue(), vk::PipelineStageFlags::Transfer)
{
    m_engine = &engine;

    m_inputImageUsage = std::make_unique<VoxelEngine::RenderGraph::ImageUsage>(*this, vk::ImageLayout::TransferSrcOptimal, vk::AccessFlags::TransferWrite, vk::PipelineStageFlags::Transfer);
    m_outputImageUsage = std::make_unique<VoxelEngine::RenderGraph::ImageUsage>(*this, vk::ImageLayout::TransferSrcOptimal, vk::AccessFlags::TransferWrite, vk::PipelineStageFlags::Transfer);
}

void MipmapGenerator::generate(std::shared_ptr<VoxelEngine::Image> image) {
    m_images.push_back(image);
}

void MipmapGenerator::preRender(uint32_t currentFrame) {
    for (auto& image : m_images) {
        vk::ImageSubresourceRange subresource = {};
        subresource.aspectMask = vk::ImageAspectFlags::Color;
        subresource.baseArrayLayer = 0;
        subresource.layerCount = image->image().arrayLayers();
        subresource.baseMipLevel = 0;
        subresource.levelCount = 1;

        //sync mip level 0 of all array layers
        m_inputImageUsage->sync(image, subresource);

        //sync all mip levels of all array layers
        subresource.levelCount = image->image().mipLevels();
        m_outputImageUsage->sync(image, subresource);
    }
}

void MipmapGenerator::render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    for (auto& image : m_images) {
        int32_t width = static_cast<int32_t>(image->image().extent().width);
        int32_t height = static_cast<int32_t>(image->image().extent().height);

        //start counting from 1, generate mips for level i from level (i - 1)
        //generate mips for all layers at once
        for (uint32_t i = 1; i < image->image().mipLevels(); i++) {
            //transition level i to TRANSFER_DST_OPTIMAL
            vk::ImageMemoryBarrier barrier = {};
            barrier.image = &image->image();
            barrier.oldLayout = vk::ImageLayout::Undefined;
            barrier.newLayout = vk::ImageLayout::TransferDstOptimal;
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlags::TransferWrite;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlags::Color;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = image->image().arrayLayers();
            barrier.subresourceRange.baseMipLevel = i;
            barrier.subresourceRange.levelCount = 1;

            commandBuffer.pipelineBarrier(vk::PipelineStageFlags::Transfer, vk::PipelineStageFlags::Transfer, {},
                nullptr,
                nullptr,
                barrier
            );

            int32_t nextWidth = width / 2;
            int32_t nextHeight = height / 2;

            vk::ImageBlit blit = {};
            blit.srcOffsets[0] = {};
            blit.srcOffsets[1] = { width, height, 1 };
            blit.srcSubresource.aspectMask = vk::ImageAspectFlags::Color;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = image->image().arrayLayers();
            blit.srcSubresource.mipLevel = i - 1;
            blit.dstOffsets[0] = {};
            blit.dstOffsets[1] = { nextWidth, nextHeight, 1 };
            blit.dstSubresource.aspectMask = vk::ImageAspectFlags::Color;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = image->image().arrayLayers();
            blit.dstSubresource.mipLevel = i;

            commandBuffer.blitImage(image->image(), vk::ImageLayout::TransferSrcOptimal, image->image(), vk::ImageLayout::TransferDstOptimal, blit, vk::Filter::Linear);

            //transition level i to TRANSFER_SRC_OPTIMAL
            barrier.oldLayout = vk::ImageLayout::TransferDstOptimal;
            barrier.newLayout = vk::ImageLayout::TransferSrcOptimal;
            barrier.srcAccessMask = vk::AccessFlags::TransferWrite;
            barrier.dstAccessMask = vk::AccessFlags::TransferRead;

            commandBuffer.pipelineBarrier(vk::PipelineStageFlags::Transfer, vk::PipelineStageFlags::Transfer, {},
                nullptr,
                nullptr,
                barrier
            );

            width = nextWidth;
            height = nextHeight;
        }
    }

    m_images.clear();
}