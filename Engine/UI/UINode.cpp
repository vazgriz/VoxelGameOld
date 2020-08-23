#include "Engine/UI/UINode.h"
#include "Engine/Engine.h"
#include "Engine/RenderGraph/RenderGraph.h"
#include "Engine/UI/Canvas.h"

static const vk::Format colorFormat = vk::Format::R8G8B8A8_Unorm;

using namespace VoxelEngine;
using namespace VoxelEngine::UI;

UINode::UINode(Engine& engine, RenderGraph& graph)
    : RenderGraph::Node(graph, *engine.getGraphics().graphicsQueue())
{
    m_engine = &engine;
    m_graphics = &engine.getGraphics();
    m_renderGraph = &graph;

    m_imageUsage = std::make_unique<RenderGraph::ImageUsage>(*this, vk::ImageLayout::ColorAttachmentOptimal, vk::AccessFlags::ShaderWrite, vk::PipelineStageFlags::ColorAttachmentOutput);

    createRenderPass();
    createDescriptorSetLayout();
}

void UINode::addCanvas(Canvas& canvas) {
    canvas.setRenderPass(*m_renderPass);
    canvas.setCameraDescriptorLayout(*m_descriptorSetLayout);
    m_canvases.push_back(&canvas);
}

void UINode::preRender(uint32_t currentFrame) {
    for (auto& canvas : m_canvases) {
        canvas->preRender();
    }
}

void UINode::render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    for (auto& canvas : m_canvases) {
        canvas->render(commandBuffer);
    }
}

void UINode::createRenderPass() {
    vk::AttachmentDescription attachment = {};
    attachment.format = colorFormat;
    attachment.samples = vk::SampleCountFlags::_1;
    attachment.loadOp = vk::AttachmentLoadOp::Clear;
    attachment.storeOp = vk::AttachmentStoreOp::Store;
    attachment.stencilLoadOp = vk::AttachmentLoadOp::DontCare;
    attachment.stencilStoreOp = vk::AttachmentStoreOp::DontCare;
    attachment.initialLayout = vk::ImageLayout::Undefined;
    attachment.finalLayout = vk::ImageLayout::ShaderReadOnlyOptimal;

    vk::AttachmentReference ref = {};
    ref.attachment = 0;
    ref.layout = vk::ImageLayout::ColorAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::Graphics;
    subpass.colorAttachments = { ref };

    vk::RenderPassCreateInfo info = {};
    info.attachments = { attachment };
    info.subpasses = { subpass };

    m_renderPass = std::make_unique<vk::RenderPass>(m_graphics->device(), info);
}

void UINode::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = vk::DescriptorType::UniformBuffer;
    binding.descriptorCount = 1;
    binding.stageFlags = vk::ShaderStageFlags::Vertex;

    vk::DescriptorSetLayoutCreateInfo info = {};
    info.bindings = { binding };

    m_descriptorSetLayout = std::make_unique<vk::DescriptorSetLayout>(m_engine->getGraphics().device(), info);
}