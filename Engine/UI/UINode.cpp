#include "Engine/UI/UINode.h"
#include "Engine/RenderGraph/RenderGraph.h"

using namespace VoxelEngine;
using namespace VoxelEngine::UI;

UINode::UINode(RenderGraph& graph, const vk::Queue& queue)
    : RenderGraph::Node(graph, queue, vk::PipelineStageFlags::TopOfPipe)
{
    m_imageUsage = std::make_unique<RenderGraph::ImageUsage>(*this, vk::ImageLayout::ColorAttachmentOptimal, vk::AccessFlags::ShaderWrite, vk::PipelineStageFlags::ColorAttachmentOutput);
}

void UINode::render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {

}