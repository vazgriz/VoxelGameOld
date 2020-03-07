#include "SelectionBox.h"
#include <Engine/Utilities.h>
#include <stb_image.h>
#include "Chunk.h"
#include "MeshManager.h"

const std::string textureName = "resources/selection.png";
const uint32_t textureSize = 16;

SelectionBox::SelectionBox(VoxelEngine::Engine& engine, VoxelEngine::CameraSystem& cameraSystem) {
    m_engine = &engine;
    m_cameraSystem = &cameraSystem;

    createTexture();
    createImageView();
    createSampler();
    createDescriptorSetLayout();
    createDescriptorPool();
    createDecriptorSet();
    writeDescriptorSet();
    createPipelineLayout();
}

void SelectionBox::transfer(VoxelEngine::TransferNode& transferNode) {
    vk::ImageSubresourceLayers subresource = {};
    subresource.aspectMask = vk::ImageAspectFlags::Color;
    subresource.layerCount = 1;
    subresource.baseArrayLayer = 0;

    int width, height, channels;
    auto data = stbi_load(textureName.c_str(), &width, &height, &channels, 4);

    transferNode.transfer(*m_image, vk::Offset3D{}, vk::Extent3D{ textureSize, textureSize, 1 }, subresource, data);

    stbi_image_free(data);
}

void SelectionBox::createMesh(VoxelEngine::TransferNode& transferNode, MeshManager& meshManager) {
    m_mesh = std::make_unique<VoxelEngine::Mesh>();

    std::vector<glm::i8vec4> positions;
    std::vector<glm::i8vec4> uvs;

    for (size_t i = 0; i < 6; i++) {
        for (size_t j = 0; j < 4; j++) {
            positions.push_back(glm::i8vec4(Chunk::NeighborFaces[i].vertices[j], 0));
            uvs.push_back(glm::i8vec4(Chunk::uvFaces[j], 0, 0));
        }
    }

    size_t vertexSize = positions.size() * sizeof(glm::i8vec4);
    size_t uvSize = uvs.size() * sizeof(glm::i8vec4);

    vk::BufferCreateInfo vertexInfo = {};
    vertexInfo.size = vertexSize;
    vertexInfo.usage = vk::BufferUsageFlags::VertexBuffer | vk::BufferUsageFlags::TransferDst;
    vertexInfo.sharingMode = vk::SharingMode::Exclusive;

    vk::BufferCreateInfo uvInfo = vertexInfo;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    std::shared_ptr<VoxelEngine::Buffer> vertexBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, vertexInfo, allocInfo);
    std::shared_ptr<VoxelEngine::Buffer> uvsBuffer = std::make_shared<VoxelEngine::Buffer>(*m_engine, uvInfo, allocInfo);

    transferNode.transfer(*vertexBuffer, vertexSize, 0, positions.data());
    transferNode.transfer(*uvsBuffer, uvSize, 0, uvs.data());

    m_mesh->addBinding(vertexBuffer, 0);
    m_mesh->addBinding(uvsBuffer, 0);
    m_mesh->setIndexBuffer(meshManager.indexBuffer(), vk::IndexType::Uint32, 0);
}

void SelectionBox::setSelection(std::optional<RaycastResult>& raycast) {
    m_selection = raycast;
}

void SelectionBox::draw(vk::CommandBuffer& commandBuffer, vk::Viewport viewport, vk::Rect2D scissor) {
    if (m_selection) {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::Graphics, *m_pipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::Graphics, *m_pipelineLayout, 0, { m_cameraSystem->descriptorSet(), *m_descriptorSet }, nullptr);
        commandBuffer.setViewport(0, { viewport });
        commandBuffer.setScissor(0, { scissor });

        glm::ivec4 data = glm::ivec4(m_selection->blockPosition, 0);
        commandBuffer.pushConstants(*m_pipelineLayout, vk::ShaderStageFlags::Vertex, 0, sizeof(glm::ivec4), &data);

        m_mesh->drawIndexed(commandBuffer, 36);
    }
}

void SelectionBox::createTexture() {
    vk::ImageCreateInfo info = {};
    info.usage = vk::ImageUsageFlags::TransferDst | vk::ImageUsageFlags::Sampled;
    info.arrayLayers = 1;
    info.mipLevels = 1;
    info.extent = { textureSize, textureSize, 1 };
    info.samples = vk::SampleCountFlags::_1;
    info.format = vk::Format::R8G8B8A8_Unorm;
    info.imageType = vk::ImageType::_2D;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_image = std::make_unique<VoxelEngine::Image>(*m_engine, info, allocInfo);
}

void SelectionBox::createImageView() {
    vk::ImageViewCreateInfo info = {};
    info.viewType = vk::ImageViewType::_2D;
    info.format = vk::Format::R8G8B8A8_Unorm;
    info.image = &m_image->image();
    info.subresourceRange.aspectMask = vk::ImageAspectFlags::Color;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;

    m_imageView = std::make_unique<vk::ImageView>(m_engine->getGraphics().device(), info);
}

void SelectionBox::createSampler() {
    vk::SamplerCreateInfo info = {};
    info.magFilter = vk::Filter::Nearest;
    info.minFilter = vk::Filter::Nearest;
    info.mipmapMode = vk::SamplerMipmapMode::Nearest;
    info.addressModeU = vk::SamplerAddressMode::ClampToEdge;
    info.addressModeV = vk::SamplerAddressMode::ClampToEdge;
    //info.anisotropyEnable = true;
    info.maxAnisotropy = 16.0f;

    m_sampler = std::make_unique<vk::Sampler>(m_engine->getGraphics().device(), info);
}

void SelectionBox::createDescriptorSetLayout() {
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

void SelectionBox::createDescriptorPool() {
    vk::DescriptorPoolCreateInfo info = {};
    info.maxSets = 1;
    info.poolSizes = {
        { vk::DescriptorType::Sampler, 1 },
        { vk::DescriptorType::SampledImage, 1 }
    };

    m_descriptorPool = std::make_unique<vk::DescriptorPool>(m_engine->getGraphics().device(), info);
}

void SelectionBox::createDecriptorSet() {
    vk::DescriptorSetAllocateInfo info = {};
    info.descriptorPool = m_descriptorPool.get();
    info.setLayouts = { *m_descriptorSetLayout };

    m_descriptorSet = std::make_unique<vk::DescriptorSet>(std::move(m_descriptorPool->allocate(info)[0]));
}

void SelectionBox::writeDescriptorSet() {
    vk::DescriptorImageInfo imageInfo1 = {};
    imageInfo1.sampler = m_sampler.get();

    vk::DescriptorImageInfo imageInfo2 = {};
    imageInfo2.imageView = m_imageView.get();
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

void SelectionBox::createPipelineLayout() {
    vk::PipelineLayoutCreateInfo info = {};
    info.setLayouts = {
        m_cameraSystem->descriptorLayout(),
        *m_descriptorSetLayout
    };
    info.pushConstantRanges = {
        {
            vk::ShaderStageFlags::Vertex,
            0,
            sizeof(glm::ivec4)
        }
    };

    m_pipelineLayout = std::make_unique<vk::PipelineLayout>(m_engine->getGraphics().device(), info);
}

void SelectionBox::createPipeline(vk::RenderPass& renderPass) {
    std::vector<char> vertShaderCode = VoxelEngine::readFile("shaders/selection.vert.spv");
    std::vector<char> fragShaderCode = VoxelEngine::readFile("shaders/selection.frag.spv");

    vk::ShaderModule vertShader = VoxelEngine::createShaderModule(m_engine->getGraphics().device(), vertShaderCode);
    vk::ShaderModule fragShader = VoxelEngine::createShaderModule(m_engine->getGraphics().device(), fragShaderCode);

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
        }
    };
    vertexInputInfo.vertexAttributeDescriptions = {
        {
            0, 0, vk::Format::R8G8B8A8_Sint
        },
        {
            1, 1, vk::Format::R8G8B8A8_Sint
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
    colorBlendState.blendEnable = true;
    colorBlendState.colorBlendOp = vk::BlendOp::Add;
    colorBlendState.srcColorBlendFactor = vk::BlendFactor::SrcAlpha;
    colorBlendState.dstColorBlendFactor = vk::BlendFactor::OneMinusSrcAlpha;

    vk::PipelineColorBlendStateCreateInfo colorBlendInfo = {};
    colorBlendInfo.attachments = { colorBlendState };

    vk::PipelineDepthStencilStateCreateInfo depthInfo = {};
    depthInfo.depthTestEnable = true;
    depthInfo.depthCompareOp = vk::CompareOp::Equal;

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
    info.renderPass = &renderPass;
    info.subpass = 0;

    m_pipeline = std::make_unique<vk::GraphicsPipeline>(m_engine->getGraphics().device(), info);
}