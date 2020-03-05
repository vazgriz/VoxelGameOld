#include "ChunkRenderer.h"
#include <glm/glm.hpp>
#include "Chunk.h"
#include "ChunkMesh.h"
#include <Engine/Utilities.h>

ChunkRenderer::ChunkRenderer(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& graph, VoxelEngine::AcquireNode& acquireNode, VoxelEngine::TransferNode& transferNode, VoxelEngine::CameraSystem& cameraSystem, World& world, TextureManager& textureManager, SkyboxManager& skyboxManager, SelectionBox& selectionBox)
    : VoxelEngine::RenderGraph::Node(graph, *engine.getGraphics().graphicsQueue(), vk::PipelineStageFlags::ColorAttachmentOutput) {
    m_engine = &engine;
    m_graphics = &engine.getGraphics();
    m_acquireNode = &acquireNode;
    m_transferNode = &transferNode;
    m_cameraSystem = &cameraSystem;
    m_world = &world;
    m_textureManager = &textureManager;
    m_skyboxManager = &skyboxManager;
    m_selectionBox = &selectionBox;

    createDepthBuffer();
    createRenderPass();
    createFramebuffers();
    createPipelineLayout();
    createPipeline();

    m_uniformBufferUsage = std::make_unique<VoxelEngine::RenderGraph::BufferUsage>(*this, vk::AccessFlags::ShaderRead, vk::PipelineStageFlags::VertexShader);
    m_vertexBufferUsage = std::make_unique<VoxelEngine::RenderGraph::BufferUsage>(*this, vk::AccessFlags::VertexAttributeRead, vk::PipelineStageFlags::VertexInput);
    m_indexBufferUsage = std::make_unique<VoxelEngine::RenderGraph::BufferUsage>(*this, vk::AccessFlags::IndexRead, vk::PipelineStageFlags::VertexInput);
    m_textureUsage = std::make_unique<VoxelEngine::RenderGraph::ImageUsage>(*this, vk::ImageLayout::ShaderReadOnlyOptimal, vk::AccessFlags::ShaderRead, vk::PipelineStageFlags::FragmentShader);
    m_skyboxUsage = std::make_unique<VoxelEngine::RenderGraph::ImageUsage>(*this, vk::ImageLayout::ShaderReadOnlyOptimal, vk::AccessFlags::ShaderRead, vk::PipelineStageFlags::FragmentShader);
    m_selectionTextureUsage = std::make_unique<VoxelEngine::RenderGraph::ImageUsage>(*this, vk::ImageLayout::ShaderReadOnlyOptimal, vk::AccessFlags::ShaderRead, vk::PipelineStageFlags::FragmentShader);
    m_imageUsage = std::make_unique<VoxelEngine::RenderGraph::ImageUsage>(*this, vk::ImageLayout::ColorAttachmentOptimal, vk::AccessFlags::ColorAttachmentWrite, vk::PipelineStageFlags::ColorAttachmentOutput);

    m_graphics->onSwapchainChanged().connect<&ChunkRenderer::onSwapchainChanged>(this);
}

void ChunkRenderer::preRender(uint32_t currentFrame) {
    m_uniformBufferUsage->sync(*m_cameraSystem->uniformBuffer(), VK_WHOLE_SIZE, 0);

    auto view = m_world->registry().view<Chunk, ChunkMesh>();
    for (auto entity : view) {
        auto& mesh = view.get<ChunkMesh>(entity);

        if (mesh.dirty()) {
            mesh.clearDirty();

            m_vertexBufferUsage->sync(*mesh.mesh().getBinding(0), VK_WHOLE_SIZE, 0);
            m_vertexBufferUsage->sync(*mesh.mesh().getBinding(1), VK_WHOLE_SIZE, 0);
            m_vertexBufferUsage->sync(*mesh.mesh().getBinding(2), VK_WHOLE_SIZE, 0);
        }
    }

    m_vertexBufferUsage->sync(*m_selectionBox->mesh().getBinding(0), VK_WHOLE_SIZE, 0);
    m_vertexBufferUsage->sync(*m_selectionBox->mesh().getBinding(1), VK_WHOLE_SIZE, 0);

    vk::ImageSubresourceRange subresource = {};
    subresource.aspectMask = vk::ImageAspectFlags::Color;
    subresource.baseArrayLayer = 0;
    subresource.layerCount = 1;
    subresource.baseMipLevel = 0;
    subresource.levelCount = 1;

    m_selectionTextureUsage->sync(m_selectionBox->image(), subresource);

    subresource.layerCount = m_textureManager->count();
    subresource.levelCount = m_textureManager->mipLevelCount();

    m_textureUsage->sync(*m_textureManager->image(), subresource);

    subresource.layerCount = 1;
    subresource.levelCount = 1;

    for (uint32_t i = 0; i < 6; i++) {
        subresource.baseArrayLayer = i;
        m_skyboxUsage->sync(m_skyboxManager->image(), subresource);
    }
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

    vk::Viewport viewport = {};
    viewport.width = static_cast<float>(m_graphics->swapchain().extent().width);
    viewport.height = static_cast<float>(m_graphics->swapchain().extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor = {};
    scissor.extent = m_graphics->swapchain().extent();

    commandBuffer.setViewport(0, { viewport });
    commandBuffer.setScissor(0, { scissor });

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::Graphics, *m_pipelineLayout, 0, { m_cameraSystem->descriptorSet(), m_textureManager->descriptorSet() }, nullptr);

    VoxelEngine::Frustum frustum = m_cameraSystem->camera().frustum();

    auto view = m_world->registry().view<Chunk, ChunkMesh>();
    for (auto entity : view) {
        auto& chunk = view.get<Chunk>(entity);
        if (chunk.loadState() != ChunkLoadState::Loaded) continue;
        if (!frustum.testAABB(chunk.worldChunkPosition() * 16, glm::vec3(16, 16, 16))) continue;

        auto& mesh = view.get<ChunkMesh>(entity);

        if (!mesh.isEmpty()) {
            glm::ivec4 transform = glm::ivec4(chunk.worldChunkPosition(), 0) * Chunk::chunkSize;
            commandBuffer.pushConstants(*m_pipelineLayout, vk::ShaderStageFlags::Vertex, 0, sizeof(glm::ivec4), &transform);
            mesh.mesh().drawIndexed(commandBuffer);
        }
    }

    m_selectionBox->draw(commandBuffer, viewport, scissor);
    m_skyboxManager->draw(commandBuffer, viewport, scissor);

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
    m_framebuffers.clear();

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
    info.setLayouts = {
        m_cameraSystem->descriptorLayout(),
        m_textureManager->descriptorSetLayout()
    };
    info.pushConstantRanges = {
        {
            vk::ShaderStageFlags::Vertex,
            0,
            sizeof(glm::ivec4)
        }
    };

    m_pipelineLayout = std::make_unique<vk::PipelineLayout>(m_graphics->device(), info);
}

void ChunkRenderer::createPipeline() {
    std::vector<char> vertShaderCode = VoxelEngine::readFile("shaders/shader.vert.spv");
    std::vector<char> fragShaderCode = VoxelEngine::readFile("shaders/shader.frag.spv");

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
    vertexInputInfo.vertexBindingDescriptions = {
        {
            0, sizeof(glm::i8vec4)
        },
        {
            1, sizeof(glm::i8vec4)
        },
        {
            2, sizeof(glm::i8vec4)
        }
    };
    vertexInputInfo.vertexAttributeDescriptions = {
        {
            0, 0, vk::Format::R8G8B8A8_Sint
        },
        {
            1, 1, vk::Format::R8G8B8A8_Unorm
        },
        {
            2, 2, vk::Format::R8G8B8A8_Sint
        },
    };

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

void ChunkRenderer::onSwapchainChanged(vk::Swapchain& swapchain) {
    createDepthBuffer();
    createFramebuffers();
}