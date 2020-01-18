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

TriangleRenderer::TriangleRenderer(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& graph, VoxelEngine::AcquireNode& acquireNode, VoxelEngine::TransferNode& transferNode, VoxelEngine::CameraSystem& cameraSystem)
    : VoxelEngine::RenderGraph::Node(graph, *engine.getGraphics().graphicsQueue(), vk::PipelineStageFlags::ColorAttachmentOutput) {
    m_engine = &engine;
    m_graphics = &engine.getGraphics();
    m_acquireNode = &acquireNode;
    m_transferNode = &transferNode;
    m_cameraSystem = &cameraSystem;

    createRenderPass();
    createFramebuffers();
    createPipelineLayout();
    createPipeline();
    createMesh();
}

void TriangleRenderer::preRender(uint32_t currentFrame) {
    sync(m_cameraSystem->uniformBuffer(), VK_WHOLE_SIZE, 0, vk::AccessFlags::ShaderRead, vk::PipelineStageFlags::VertexShader);
}

void TriangleRenderer::render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_renderPass.get();
    renderPassInfo.framebuffer = &m_framebuffers[m_acquireNode->swapchainIndex()];
    renderPassInfo.renderArea.extent = m_graphics->swapchain().extent();
    renderPassInfo.clearValues = { { 0.0f, 0.0f, 0.0f, 1.0f } };

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::Inline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::Graphics, *m_pipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::Graphics, *m_pipelineLayout, 0, { m_cameraSystem->descriptorSet() }, nullptr);

    m_mesh->draw(commandBuffer);

    commandBuffer.endRenderPass();
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

void TriangleRenderer::createPipelineLayout() {
    vk::PipelineLayoutCreateInfo info = {};
    info.setLayouts = { m_cameraSystem->descriptorLayout() };

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
    vertexInputInfo.vertexBindingDescriptions = {
        {
            0, sizeof(glm::vec3)
        },
        {
            1, sizeof(glm::vec3)
        }
    };
    vertexInputInfo.vertexAttributeDescriptions = {
        {
            0, 0, vk::Format::R32G32B32_Sfloat
        },
        {
            1, 1, vk::Format::R32G32B32_Sfloat
        }
    };

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
    { 1, -1, 0},
    { -1, -1, 0}
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

    m_transferNode->transfer(*vertexBuffer, vertexSize, 0, vertices.data());
    m_transferNode->transfer(*colorBuffer, colorSize, 0, colors.data());
    m_transferNode->transfer(*indexBuffer, indexSize, 0, indices.data());

    sync(*vertexBuffer, vertexSize, 0, vk::AccessFlags::TransferRead | vk::AccessFlags::VertexAttributeRead, vk::PipelineStageFlags::VertexInput);
    sync(*colorBuffer, colorSize, 0, vk::AccessFlags::TransferRead | vk::AccessFlags::VertexAttributeRead, vk::PipelineStageFlags::VertexInput);
    sync(*indexBuffer, indexSize, 0, vk::AccessFlags::TransferRead | vk::AccessFlags::IndexRead, vk::PipelineStageFlags::VertexInput);
}