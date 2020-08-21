#include "CompositorNode.h"
#include <Engine/Utilities.h>

CompositorNode::CompositorNode(
    VoxelEngine::Engine& engine,
    VoxelEngine::RenderGraph& renderGraph,
    VoxelEngine::AcquireNode& acquireNode,
    ChunkRenderer& chunkRenderer
)
    : VoxelEngine::RenderGraph::Node(renderGraph, *engine.getGraphics().graphicsQueue())
{
    m_engine = &engine;
    m_graphics = &engine.getGraphics();
    m_acquireNode = &acquireNode;
    m_chunkRenderer = &chunkRenderer;

    m_mainUsage = std::make_unique<VoxelEngine::RenderGraph::ImageUsage>(*this, vk::ImageLayout::ShaderReadOnlyOptimal, vk::AccessFlags::ShaderRead, vk::PipelineStageFlags::FragmentShader);
    m_uiUsage = std::make_unique<VoxelEngine::RenderGraph::ImageUsage>(*this, vk::ImageLayout::ShaderReadOnlyOptimal, vk::AccessFlags::ShaderRead, vk::PipelineStageFlags::FragmentShader);

    createRenderPass();
    createFramebuffers();
    createDescriptorSetLayout();
    createPipelineLayout();
    createPipeline();
    createSampler();
    createDescriptorPool();
    createDescriptorSet();
    writeDescriptorSet();

    m_graphics->onSwapchainChanged().connect<&CompositorNode::onSwapchainChanged>(this);
}

void CompositorNode::preRender(uint32_t currentFrame) {
    vk::ImageSubresourceRange subresource = {};
    subresource.aspectMask = vk::ImageAspectFlags::Color;
    subresource.baseArrayLayer = 0;
    subresource.layerCount = 1;
    subresource.baseMipLevel = 0;
    subresource.levelCount = 1;

    m_mainUsage->sync(m_chunkRenderer->colorBuffer(), subresource);
};

void CompositorNode::render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_renderPass.get();
    renderPassInfo.framebuffer = &m_framebuffers[m_acquireNode->swapchainIndex()];
    renderPassInfo.renderArea.extent = m_graphics->swapchain().extent();
    renderPassInfo.clearValues.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::Inline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::Graphics, *m_pipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::Graphics, *m_pipelineLayout, 0, { *m_descriptorSet }, nullptr);

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
    info.setLayouts = {
        *m_descriptorSetLayout
    };

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

void CompositorNode::createSampler() {
    vk::SamplerCreateInfo info = {};
    info.magFilter = vk::Filter::Nearest;
    info.minFilter = vk::Filter::Nearest;
    info.addressModeU = vk::SamplerAddressMode::ClampToEdge;
    info.addressModeV = vk::SamplerAddressMode::ClampToEdge;

    m_sampler = std::make_unique<vk::Sampler>(m_engine->getGraphics().device(), info);
}

void CompositorNode::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding binding1 = {};
    binding1.binding = 0;
    binding1.descriptorType = vk::DescriptorType::Sampler;
    binding1.descriptorCount = 1;
    binding1.stageFlags = vk::ShaderStageFlags::Fragment;

    vk::DescriptorSetLayoutBinding binding2 = {};
    binding2.binding = 1;
    binding2.descriptorType = vk::DescriptorType::SampledImage;
    binding2.descriptorCount = 1;
    binding2.stageFlags = vk::ShaderStageFlags::Fragment;

    vk::DescriptorSetLayoutCreateInfo info = {};
    info.bindings = {
        binding1,
        binding2
    };

    m_descriptorSetLayout = std::make_unique<vk::DescriptorSetLayout>(m_engine->getGraphics().device(), info);
}

void CompositorNode::createDescriptorPool() {
    vk::DescriptorPoolCreateInfo info = {};
    info.maxSets = 1;
    info.poolSizes = {
        { vk::DescriptorType::Sampler, 1 },
        { vk::DescriptorType::SampledImage, 1 }
    };

    m_descriptorPool = std::make_unique<vk::DescriptorPool>(m_engine->getGraphics().device(), info);
}

void CompositorNode::createDescriptorSet() {
    vk::DescriptorSetAllocateInfo info = {};
    info.descriptorPool = m_descriptorPool.get();
    info.setLayouts = { *m_descriptorSetLayout };

    m_descriptorSet = std::make_unique<vk::DescriptorSet>(std::move(m_descriptorPool->allocate(info)[0]));
}

void CompositorNode::writeDescriptorSet() {
    vk::DescriptorImageInfo imageInfo1 = {};
    imageInfo1.sampler = m_sampler.get();

    vk::DescriptorImageInfo imageInfo2 = {};
    imageInfo2.imageView = &m_chunkRenderer->colorBufferView();
    imageInfo2.imageLayout = vk::ImageLayout::ShaderReadOnlyOptimal;

    vk::WriteDescriptorSet write1 = {};
    write1.imageInfo = { imageInfo1 };
    write1.dstSet = m_descriptorSet.get();
    write1.dstBinding = 0;
    write1.descriptorType = vk::DescriptorType::Sampler;

    vk::WriteDescriptorSet write2 = {};
    write2.imageInfo = { imageInfo2 };
    write2.dstSet = m_descriptorSet.get();
    write2.dstBinding = 1;
    write2.descriptorType = vk::DescriptorType::SampledImage;

    m_descriptorSet->update(m_engine->getGraphics().device(), { write1, write2 }, nullptr);
}

void CompositorNode::onSwapchainChanged(vk::Swapchain& swapchain) {
    createRenderPass();
    createFramebuffers();
    writeDescriptorSet();
}