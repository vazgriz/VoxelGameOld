#include "TriangleRenderer.h"
#include <glm/glm.hpp>
#include <fstream>
#include <iostream>

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

TriangleRenderer::TriangleRenderer(uint32_t priority, VoxelEngine::Engine& engine, VoxelEngine::RenderSystem& renderSystem) : System(priority) {
    m_engine = &engine;
    m_graphics = &engine.getGraphics();
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
    createMesh();
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

void TriangleRenderer::update(VoxelEngine::Clock& clock) {
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

    m_mesh->draw(commandBuffer);

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

static std::vector<glm::vec3> vertices = {
    { 0, 1, 0 },
    { 1, 0, 0},
    { -1, 0, 0}
};

static std::vector<glm::vec3> colors = {
    { 1, 0, 0 },
    { 0, 1, 0 },
    { 0, 0, 1 }
};

static std::vector<uint32_t> indices = {
    0, 1, 2
};

void TriangleRenderer::createMesh() {
    m_mesh = std::make_unique<VoxelEngine::Mesh>();

    size_t vertexSize = vertices.size() * sizeof(glm::vec3);
    size_t colorSize = colors.size() * sizeof(glm::vec3);
    size_t indexSize = indices.size() * sizeof(uint32_t);

    vk::BufferCreateInfo vertexInfo = {};
    vertexInfo.size = vertexSize;
    vertexInfo.usage = vk::BufferUsageFlags::VertexBuffer | vk::BufferUsageFlags::TransferDst;
    vertexInfo.sharingMode = vk::SharingMode::Exclusive;

    vk::BufferCreateInfo colorInfo = vertexInfo;
    colorInfo.size = colorSize;

    vk::BufferCreateInfo indexInfo = vertexInfo;
    indexInfo.size = indexSize;
    indexInfo.usage = vk::BufferUsageFlags::IndexBuffer | vk::BufferUsageFlags::TransferDst;;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.flags = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    std::shared_ptr<VoxelEngine::Buffer> vertexBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, vertexInfo, allocInfo);
    std::shared_ptr<VoxelEngine::Buffer> colorBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, colorInfo, allocInfo);
    std::shared_ptr<VoxelEngine::Buffer> indexBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, indexInfo, allocInfo);

    m_mesh->addBinding(3, vertexBuffer, vk::Format::R32G32B32A32_Sfloat);
    m_mesh->addBinding(3, colorBuffer, vk::Format::R32G32B32A32_Sfloat);

    m_mesh->setIndexBuffer(3, indexBuffer, vk::IndexType::Uint32, 0);

    transferMesh(vertexBuffer, colorBuffer, indexBuffer);
}

void TriangleRenderer::transferMesh(const std::shared_ptr<VoxelEngine::Buffer>& vertexBuffer, const std::shared_ptr<VoxelEngine::Buffer>& colorBuffer, const std::shared_ptr<VoxelEngine::Buffer>& indexBuffer) {
    size_t vertexSize = vertices.size() * sizeof(glm::vec3);
    size_t colorSize = colors.size() * sizeof(glm::vec3);
    size_t indexSize = indices.size() * sizeof(uint32_t);

    vk::BufferCreateInfo stagingInfo = {};
    stagingInfo.size = vertexSize + colorSize + indexSize;
    stagingInfo.usage = vk::BufferUsageFlags::TransferSrc;
    stagingInfo.sharingMode = vk::SharingMode::Exclusive;

    VmaAllocationCreateInfo stagingAllocInfo = {};
    stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    stagingAllocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VoxelEngine::Buffer stagingBuffer(*m_engine, stagingInfo, stagingAllocInfo);

    void* mapping = stagingBuffer.getMapping();
    memcpy(mapping, vertices.data(), vertexSize);
    memcpy(static_cast<char*>(mapping) + vertexSize, colors.data(), colorSize);
    memcpy(static_cast<char*>(mapping) + vertexSize + colorSize, indices.data(), indexSize);

    vk::CommandBufferAllocateInfo info = {};
    info.commandPool = m_commandPool.get();
    info.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer = std::move(m_commandPool->allocate(info)[0]);

    vk::CommandBufferBeginInfo beginInfo = {};
    beginInfo.flags = vk::CommandBufferUsageFlags::OneTimeSubmit;

    commandBuffer.begin(beginInfo);

    vk::BufferCopy vertexCopy = {};
    vertexCopy.size = vertexSize;

    vk::BufferCopy colorCopy = {};
    colorCopy.size = colorSize;
    colorCopy.srcOffset = vertexSize;

    vk::BufferCopy indexCopy = {};
    indexCopy.size = indexSize;
    indexCopy.srcOffset = vertexSize + colorSize;

    commandBuffer.copyBuffer(stagingBuffer.buffer(), vertexBuffer->buffer(), vertexCopy);
    commandBuffer.copyBuffer(stagingBuffer.buffer(), colorBuffer->buffer(), colorCopy);
    commandBuffer.copyBuffer(stagingBuffer.buffer(), indexBuffer->buffer(), indexCopy);

    commandBuffer.end();

    vk::SubmitInfo submitInfo = {};
    submitInfo.commandBuffers = { commandBuffer };

    m_engine->getGraphics().graphicsQueue()->submit(submitInfo, nullptr);
    m_engine->getGraphics().graphicsQueue()->waitIdle();
}