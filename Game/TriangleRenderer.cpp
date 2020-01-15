#include "TriangleRenderer.h"
#include <fstream>

std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
}

TriangleRenderer::TriangleRenderer(uint32_t priority, Graphics& renderer, RenderSystem& renderSystem) : System(priority) {
    m_graphics = &renderer;
    m_renderSystem = &renderSystem;

    vk::CommandPoolCreateInfo info = {};
    info.queueFamilyIndex = m_graphics->graphicsQueue()->familyIndex();
    info.flags = vk::CommandPoolCreateFlags::ResetCommandBuffer;

    m_commandPool = std::make_unique<vk::CommandPool>(m_graphics->device(), info);

    vk::CommandBufferAllocateInfo bufferInfo = {};
    bufferInfo.commandPool = m_commandPool.get();
    bufferInfo.commandBufferCount = static_cast<uint32_t>(m_graphics->swapchain().images().size());

    m_commandBuffers = m_commandPool->allocate(bufferInfo);

    createRenderPass();
    createFramebuffers();
    createPipelineLayout();
    createPipeline();
}

void TriangleRenderer::createRenderPass() {
    vk::AttachmentDescription attachment = {};
    attachment.format = m_graphics->swapchain().format();
    attachment.samples = vk::SampleCountFlags::_1;
    attachment.loadOp = vk::AttachmentLoadOp::DontCare;
    attachment.storeOp = vk::AttachmentStoreOp::Store;
    attachment.stencilLoadOp = vk::AttachmentLoadOp::DontCare;
    attachment.stencilStoreOp = vk::AttachmentStoreOp::DontCare;
    attachment.initialLayout = vk::ImageLayout::Undefined;
    attachment.finalLayout = vk::ImageLayout::PresentSrcKhr;

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

void TriangleRenderer::createFramebuffers() {
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

void TriangleRenderer::update(Clock& clock) {
    uint32_t index = m_renderSystem->getIndex();
    vk::CommandBuffer& commandBuffer = m_commandBuffers[index];

    commandBuffer.reset(vk::CommandBufferResetFlags::None);

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlags::OneTimeSubmit;

    commandBuffer.begin(beginInfo);

    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_renderPass.get();
    renderPassInfo.framebuffer = &m_framebuffers[index];
    renderPassInfo.renderArea.extent = m_graphics->swapchain().extent();
    renderPassInfo.clearValues = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::Inline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::Graphics, *m_pipeline);
    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();
    commandBuffer.end();

    m_renderSystem->submit(commandBuffer);
}

void TriangleRenderer::createPipelineLayout() {
    vk::PipelineLayoutCreateInfo info = {};

    m_pipelineLayout = std::make_unique<vk::PipelineLayout>(m_graphics->device(), info);
}

vk::ShaderModule createShaderModule(vk::Device& device, const std::vector<char>& byteCode) {
    vk::ShaderModuleCreateInfo info = {};
    info.code = byteCode;

    return vk::ShaderModule(device, info);
}

void TriangleRenderer::createPipeline() {
    std::vector<char> vertShaderCode = readFile("shaders/shader.vert.spv");
    std::vector<char> fragShaderCode = readFile("shaders/shader.frag.spv");

    vk::ShaderModule vertShader = createShaderModule(m_graphics->device(), vertShaderCode);
    vk::ShaderModule fragShader = createShaderModule(m_graphics->device(), fragShaderCode);

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

    vk::Viewport viewport = {};
    viewport.width = static_cast<float>(m_graphics->swapchain().extent().width);
    viewport.height = static_cast<float>(m_graphics->swapchain().extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor = {};
    scissor.extent = m_graphics->swapchain().extent();

    vk::PipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.viewports = { viewport };
    viewportInfo.scissors = { scissor };

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

    vk::GraphicsPipelineCreateInfo info = {};
    info.stages = stages;
    info.vertexInputState = &vertexInputInfo;
    info.inputAssemblyState = &inputAssemblyInfo;
    info.viewportState = &viewportInfo;
    info.rasterizationState = &rasterizerInfo;
    info.multisampleState = &multisampleInfo;
    info.colorBlendState = &colorBlendInfo;
    info.layout = m_pipelineLayout.get();
    info.renderPass = m_renderPass.get();
    info.subpass = 0;

    m_pipeline = std::make_unique<vk::GraphicsPipeline>(m_graphics->device(), info);
}