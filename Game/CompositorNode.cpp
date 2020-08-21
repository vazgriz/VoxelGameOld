#include "CompositorNode.h"
#include <Engine/Utilities.h>

CompositorNode::CompositorNode(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& renderGraph, VoxelEngine::AcquireNode& acquireNode)
    : VoxelEngine::RenderGraph::Node(renderGraph, *engine.getGraphics().graphicsQueue(), vk::PipelineStageFlags::ColorAttachmentOutput)
{
    m_engine = &engine;
    m_graphics = &engine.getGraphics();
    m_acquireNode = &acquireNode;

    m_mainUsage = std::make_unique<VoxelEngine::RenderGraph::ImageUsage>(*this, vk::ImageLayout::ShaderReadOnlyOptimal, vk::AccessFlags::ShaderRead, vk::PipelineStageFlags::FragmentShader);
    m_uiUsage = std::make_unique<VoxelEngine::RenderGraph::ImageUsage>(*this, vk::ImageLayout::ShaderReadOnlyOptimal, vk::AccessFlags::ShaderRead, vk::PipelineStageFlags::FragmentShader);

    createRenderPass();
    createFramebuffers();
    createPipelineLayout();
    createPipeline();
}

void CompositorNode::render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_renderPass.get();
    renderPassInfo.framebuffer = &m_framebuffers[m_acquireNode->swapchainIndex()];
    renderPassInfo.renderArea.extent = m_graphics->swapchain().extent();
    renderPassInfo.clearValues.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::Inline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::Graphics, *m_pipeline);

    vk::Viewport viewport = {};
    viewport.width = static_cast<float>(m_graphics->swapchain().extent().width);
    viewport.height = static_cast<float>(m_graphics->swapchain().extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor = {};
    scissor.extent = m_graphics->swapchain().extent();

    commandBuffer.setViewport(0, { viewport });
    commandBuffer.setScissor(0, { scissor });

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();
}

void CompositorNode::createRenderPass() {
    vk::AttachmentDescription attachment = {};
    attachment.format = m_graphics->swapchain().format();
    attachment.samples = vk::SampleCountFlags::_1;
    attachment.loadOp = vk::AttachmentLoadOp::Clear;
    attachment.storeOp = vk::AttachmentStoreOp::Store;
    attachment.stencilLoadOp = vk::AttachmentLoadOp::DontCare;
    attachment.stencilStoreOp = vk::AttachmentStoreOp::DontCare;
    attachment.initialLayout = vk::ImageLayout::Undefined;
    attachment.finalLayout = vk::ImageLayout::PresentSrcKHR;

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

void CompositorNode::createFramebuffers() {
    m_framebuffers.clear();

    for (size_t i = 0; i < m_graphics->swapchain().images().size(); i++) {
        vk::FramebufferCreateInfo info = {};
        info.renderPass = m_renderPass.get();
        info.attachments = { m_graphics->swapchainImageViews()[i] };
        info.width = m_graphics->swapchain().extent().width;
        info.height = m_graphics->swapchain().extent().height;
        info.layers = 1;

        m_framebuffers.emplace_back(m_graphics->device(), info);
    }
}

void CompositorNode::createPipelineLayout() {
    vk::PipelineLayoutCreateInfo info = {};

    m_pipelineLayout = std::make_unique<vk::PipelineLayout>(m_graphics->device(), info);
}

void CompositorNode::createPipeline() {
    std::vector<char> vertShaderCode = VoxelEngine::readFile("shaders/composite.vert.spv");
    std::vector<char> fragShaderCode = VoxelEngine::readFile("shaders/composite.frag.spv");

    vk::ShaderModule vertShader = VoxelEngine::createShaderModule(m_graphics->device(), vertShaderCode);
    vk::ShaderModule fragShader = VoxelEngine::createShaderModule(m_graphics->device(), fragShaderCode);

    vk::PipelineShaderStageCreateInfo vertInfo = {};
    vertInfo.module = &vertShader;
    vertInfo.name = "main";
    vertInfo.stage = vk::ShaderStageFlags::Vertex;

    vk::PipelineShaderStageCreateInfo fragInfo = {};
    fragInfo.module = &fragShader;
    fragInfo.name = "main";
    fragInfo.stage = vk::ShaderStageFlags::Fragment;

    std::vector<vk::PipelineShaderStageCreateInfo> stages = { std::move(vertInfo), std::move(fragInfo) };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.topology = vk::PrimitiveTopology::TriangleList;

    vk::PipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.viewports = { {} };    //still need 1 viewport and scissor here, even though they are dynamic
    viewportInfo.scissors = { {} };

    vk::PipelineRasterizationStateCreateInfo rasterizerInfo = {};
    rasterizerInfo.polygonMode = vk::PolygonMode::Fill;
    rasterizerInfo.lineWidth = 1.0f;
    rasterizerInfo.cullMode = vk::CullModeFlags::Back;
    rasterizerInfo.frontFace = vk::FrontFace::Clockwise;

    vk::PipelineMultisampleStateCreateInfo multisampleInfo = {};
    multisampleInfo.rasterizationSamples = vk::SampleCountFlags::_1;
    multisampleInfo.minSampleShading = 1.0f;

    vk::PipelineColorBlendAttachmentState colorBlendState = {};
    colorBlendState.colorWriteMask =
        vk::ColorComponentFlags::R
        | vk::ColorComponentFlags::G
        | vk::ColorComponentFlags::B
        | vk::ColorComponentFlags::A;

    vk::PipelineColorBlendStateCreateInfo colorBlendInfo = {};
    colorBlendInfo.attachments = { colorBlendState };

    vk::PipelineDepthStencilStateCreateInfo depthInfo = {};
    depthInfo.depthTestEnable = true;
    depthInfo.depthWriteEnable = true;
    depthInfo.depthCompareOp = vk::CompareOp::Less;

    vk::PipelineDynamicStateCreateInfo dynamicInfo = {};
    dynamicInfo.dynamicStates = { vk::DynamicState::Viewport, vk::DynamicState::Scissor };

    vk::GraphicsPipelineCreateInfo info = {};
    info.stages = stages;
    info.vertexInputState = &vertexInputInfo;
    info.inputAssemblyState = &inputAssemblyInfo;
    info.viewportState = &viewportInfo;
    info.rasterizationState = &rasterizerInfo;
    info.multisampleState = &multisampleInfo;
    info.colorBlendState = &colorBlendInfo;
    info.depthStencilState = &depthInfo;
    info.dynamicState = &dynamicInfo;
    info.layout = m_pipelineLayout.get();
    info.renderPass = m_renderPass.get();
    info.subpass = 0;

    m_pipeline = std::make_unique<vk::GraphicsPipeline>(m_graphics->device(), info);
}