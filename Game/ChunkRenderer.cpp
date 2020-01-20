#include "ChunkRenderer.h"
#include <glm/glm.hpp>
#include <fstream>
#include <iostream>

static std::vector<char> readFile(const std::string& filename) {
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

ChunkRenderer::ChunkRenderer(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& graph, VoxelEngine::AcquireNode& acquireNode, VoxelEngine::TransferNode& transferNode, VoxelEngine::CameraSystem& cameraSystem, Chunk& chunk)
    : VoxelEngine::RenderGraph::Node(graph, *engine.getGraphics().graphicsQueue(), vk::PipelineStageFlags::ColorAttachmentOutput) {
    m_engine = &engine;
    m_graphics = &engine.getGraphics();
    m_acquireNode = &acquireNode;
    m_transferNode = &transferNode;
    m_cameraSystem = &cameraSystem;
    m_chunk = &chunk;

    createDepthBuffer();
    createRenderPass();
    createFramebuffers();
    createPipelineLayout();
    createPipeline();
    createMesh();
    transferMesh();
}

void ChunkRenderer::preRender(uint32_t currentFrame) {
    sync(m_cameraSystem->uniformBuffer(), VK_WHOLE_SIZE, 0, vk::AccessFlags::ShaderRead, vk::PipelineStageFlags::VertexShader);
}

void ChunkRenderer::render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    vk::RenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.renderPass = m_renderPass.get();
    renderPassInfo.framebuffer = &m_framebuffers[m_acquireNode->swapchainIndex()];
    renderPassInfo.renderArea.extent = m_graphics->swapchain().extent();
    renderPassInfo.clearValues.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });

    vk::ClearValue depthClear = {};
    depthClear.depthStencil.depth = 1;
    renderPassInfo.clearValues.push_back(depthClear);

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::Inline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::Graphics, *m_pipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::Graphics, *m_pipelineLayout, 0, { m_cameraSystem->descriptorSet() }, nullptr);

    m_mesh->draw(commandBuffer);

    commandBuffer.endRenderPass();
}

void ChunkRenderer::createDepthBuffer() {
    vk::Extent2D extent = m_engine->getGraphics().swapchain().extent();
    vk::ImageCreateInfo info = {};
    info.extent = { extent.width, extent.height, 1 };
    info.format = vk::Format::D32_Sfloat;
    info.usage = vk::ImageUsageFlags::DepthStencilAttachment;
    info.mipLevels = 1;
    info.arrayLayers = 1;
    info.samples = vk::SampleCountFlags::_1;
    info.imageType = vk::ImageType::_2D;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_depthBuffer = std::make_unique<VoxelEngine::Image>(*m_engine, info, allocInfo);

    vk::ImageViewCreateInfo viewInfo = {};
    viewInfo.image = &m_depthBuffer->image();
    viewInfo.format = m_depthBuffer->image().format();
    viewInfo.viewType = vk::ImageViewType::_2D;
    viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlags::Depth;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;

    m_depthBufferView = std::make_unique<vk::ImageView>(m_engine->getGraphics().device(), viewInfo);
}

void ChunkRenderer::createRenderPass() {
    vk::AttachmentDescription attachment = {};
    attachment.format = m_graphics->swapchain().format();
    attachment.samples = vk::SampleCountFlags::_1;
    attachment.loadOp = vk::AttachmentLoadOp::Clear;
    attachment.storeOp = vk::AttachmentStoreOp::Store;
    attachment.stencilLoadOp = vk::AttachmentLoadOp::DontCare;
    attachment.stencilStoreOp = vk::AttachmentStoreOp::DontCare;
    attachment.initialLayout = vk::ImageLayout::Undefined;
    attachment.finalLayout = vk::ImageLayout::PresentSrcKhr;

    vk::AttachmentDescription depthAttachment = {};
    depthAttachment.format = m_depthBuffer->image().format();
    depthAttachment.samples = vk::SampleCountFlags::_1;
    depthAttachment.loadOp = vk::AttachmentLoadOp::Clear;
    depthAttachment.storeOp = vk::AttachmentStoreOp::DontCare;
    depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::DontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::DontCare;
    depthAttachment.initialLayout = vk::ImageLayout::Undefined;
    depthAttachment.finalLayout = vk::ImageLayout::DepthStencilAttachmentOptimal;

    vk::AttachmentReference ref = {};
    ref.attachment = 0;
    ref.layout = vk::ImageLayout::ColorAttachmentOptimal;

    vk::AttachmentReference depthRef = {};
    depthRef.attachment = 1;
    depthRef.layout = vk::ImageLayout::DepthStencilAttachmentOptimal;

    vk::SubpassDescription subpass = {};
    subpass.pipelineBindPoint = vk::PipelineBindPoint::Graphics;
    subpass.colorAttachments = { ref };
    subpass.depthStencilAttachment = &depthRef;

    vk::RenderPassCreateInfo info = {};
    info.attachments = { attachment, depthAttachment };
    info.subpasses = { subpass };

    m_renderPass = std::make_unique<vk::RenderPass>(m_graphics->device(), info);
}

void ChunkRenderer::createFramebuffers() {
    for (size_t i = 0; i < m_graphics->swapchain().images().size(); i++) {
        vk::FramebufferCreateInfo info = {};
        info.renderPass = m_renderPass.get();
        info.attachments = { m_graphics->swapchainImageViews()[i], *m_depthBufferView };
        info.width = m_graphics->swapchain().extent().width;
        info.height = m_graphics->swapchain().extent().height;
        info.layers = 1;

        m_framebuffers.emplace_back(m_graphics->device(), info);
    }
}

void ChunkRenderer::createPipelineLayout() {
    vk::PipelineLayoutCreateInfo info = {};
    info.setLayouts = { m_cameraSystem->descriptorLayout() };

    m_pipelineLayout = std::make_unique<vk::PipelineLayout>(m_graphics->device(), info);
}

static vk::ShaderModule createShaderModule(vk::Device& device, const std::vector<char>& byteCode) {
    vk::ShaderModuleCreateInfo info = {};
    info.code = byteCode;

    return vk::ShaderModule(device, info);
}

void ChunkRenderer::createPipeline() {
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
            1, sizeof(glm::i8vec4)
        }
    };
    vertexInputInfo.vertexAttributeDescriptions = {
        {
            0, 0, vk::Format::R32G32B32_Sint
        },
        {
            1, 1, vk::Format::R8G8B8A8_Unorm
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

    vk::PipelineDepthStencilStateCreateInfo depthInfo = {};
    depthInfo.depthTestEnable = true;
    depthInfo.depthWriteEnable = true;
    depthInfo.depthCompareOp = vk::CompareOp::Less;

    vk::GraphicsPipelineCreateInfo info = {};
    info.stages = stages;
    info.vertexInputState = &vertexInputInfo;
    info.inputAssemblyState = &inputAssemblyInfo;
    info.viewportState = &viewportInfo;
    info.rasterizationState = &rasterizerInfo;
    info.multisampleState = &multisampleInfo;
    info.colorBlendState = &colorBlendInfo;
    info.depthStencilState = &depthInfo;
    info.layout = m_pipelineLayout.get();
    info.renderPass = m_renderPass.get();
    info.subpass = 0;

    m_pipeline = std::make_unique<vk::GraphicsPipeline>(m_graphics->device(), info);
}

void ChunkRenderer::createMesh() {
    uint32_t index = 0;

    for (glm::ivec3 pos : Chunk::Positions()) {
        Block block = m_chunk->blocks()[pos];
        if (block.type == 0) continue;

        for (size_t i = 0; i < Chunk::Neighbors6.size(); i++) {
            glm::ivec3 offset = Chunk::Neighbors6[i];
            glm::ivec3 neighborPos = pos + offset;

            if (((neighborPos.x < 0 || neighborPos.x >= Chunk::chunkSize)
                || (neighborPos.y < 0 || neighborPos.y >= Chunk::chunkSize)
                || (neighborPos.z < 0 || neighborPos.z >= Chunk::chunkSize))
                || m_chunk->blocks()[neighborPos].type == 0)
            {
                Chunk::FaceArray& faceArray = Chunk::NeighborFaces[i];
                for (size_t j = 0; j < faceArray.size(); j++) {
                    m_vertexData.push_back(pos + faceArray[j]);
                    m_colorData.push_back(glm::i8vec4(pos.x * 16, pos.y * 16, pos.z * 16, 0));
                }

                m_indexData.push_back(index + 0);
                m_indexData.push_back(index + 1);
                m_indexData.push_back(index + 2);
                m_indexData.push_back(index + 1);
                m_indexData.push_back(index + 3);
                m_indexData.push_back(index + 2);
                index += 4;
            }
        }
    }
}

void ChunkRenderer::transferMesh() {
    m_mesh = std::make_unique<VoxelEngine::Mesh>();

    size_t vertexSize = m_vertexData.size() * sizeof(glm::ivec3);
    size_t colorSize = m_colorData.size() * sizeof(glm::i8vec4);
    size_t indexSize = m_indexData.size() * sizeof(uint32_t);

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

    m_mesh->addBinding(static_cast<uint32_t>(m_vertexData.size()), vertexBuffer, vk::Format::R32G32B32_Sint);
    m_mesh->addBinding(static_cast<uint32_t>(m_colorData.size()), colorBuffer, vk::Format::R8G8B8A8_Unorm);

    m_mesh->setIndexBuffer(static_cast<uint32_t>(m_indexData.size()), indexBuffer, vk::IndexType::Uint32, 0);

    m_transferNode->transfer(*vertexBuffer, vertexSize, 0, m_vertexData.data());
    m_transferNode->transfer(*colorBuffer, colorSize, 0, m_colorData.data());
    m_transferNode->transfer(*indexBuffer, indexSize, 0, m_indexData.data());

    sync(*vertexBuffer, vertexSize, 0, vk::AccessFlags::TransferRead | vk::AccessFlags::VertexAttributeRead, vk::PipelineStageFlags::VertexInput);
    sync(*colorBuffer, colorSize, 0, vk::AccessFlags::TransferRead | vk::AccessFlags::VertexAttributeRead, vk::PipelineStageFlags::VertexInput);
    sync(*indexBuffer, indexSize, 0, vk::AccessFlags::TransferRead | vk::AccessFlags::IndexRead, vk::PipelineStageFlags::VertexInput);
}