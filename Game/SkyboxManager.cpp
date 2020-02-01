#include "SkyboxManager.h"
#include <stb_image.h>
#include <Engine/math.h>
#include <fstream>

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

static const std::vector<std::string> textures = {
    "resources/sky_right.png",
    "resources/sky_left.png",
    "resources/sky_up.png",
    "resources/sky_down.png",
    "resources/sky_front.png",
    "resources/sky_back.png",
};

static const uint32_t textureSize = 1024;

SkyboxManager::SkyboxManager(VoxelEngine::Engine& engine, VoxelEngine::CameraSystem& cameraSystem) {
    m_engine = &engine;
    m_cameraSystem = &cameraSystem;

    createCubeMap();
    createImageView();
    createSampler();
    createDescriptorSetLayout();
    createDescriptorPool();
    createDecriptorSet();
    writeDescriptorSet();
    createPipelineLayout();
}

void SkyboxManager::draw(vk::CommandBuffer& commandBuffer, vk::Viewport viewport, vk::Rect2D scissor) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::Graphics, *m_pipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::Graphics, *m_pipelineLayout, 0, { m_cameraSystem->descriptorSet(), *m_descriptorSet }, nullptr);
    commandBuffer.setViewport(0, { viewport });
    commandBuffer.setScissor(0, { scissor });
    commandBuffer.draw(3, 1, 0, 0);
}

void SkyboxManager::transfer(VoxelEngine::TransferNode& transferNode) {
    vk::ImageSubresourceLayers subresource = {};
    subresource.aspectMask = vk::ImageAspectFlags::Color;
    subresource.layerCount = 1;

    for (uint32_t i = 0; i < textures.size(); i++) {
        auto& name = textures[i];
        int width, height, channels;
        auto data = stbi_load(name.c_str(), &width, &height, &channels, 4);

        subresource.baseArrayLayer = i;

        transferNode.transfer(*m_image, vk::Offset3D{}, vk::Extent3D{ textureSize, textureSize, 1 }, subresource, data);

        stbi_image_free(data);
    }
}

void SkyboxManager::createCubeMap() {
    vk::ImageCreateInfo info = {};
    info.usage = vk::ImageUsageFlags::TransferDst | vk::ImageUsageFlags::Sampled;
    info.flags = vk::ImageCreateFlags::CubeCompatible;
    info.arrayLayers = 6;
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

void SkyboxManager::createImageView() {
    vk::ImageViewCreateInfo info = {};
    info.viewType = vk::ImageViewType::Cube;
    info.format = vk::Format::R8G8B8A8_Unorm;
    info.image = &m_image->image();
    info.subresourceRange.aspectMask = vk::ImageAspectFlags::Color;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 6;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;

    m_imageView = std::make_unique<vk::ImageView>(m_engine->getGraphics().device(), info);
}

void SkyboxManager::createSampler() {
    vk::SamplerCreateInfo info = {};
    info.magFilter = vk::Filter::Nearest;
    info.minFilter = vk::Filter::Nearest;
    info.mipmapMode = vk::SamplerMipmapMode::Linear;
    info.addressModeU = vk::SamplerAddressMode::ClampToEdge;
    info.addressModeV = vk::SamplerAddressMode::ClampToEdge;
    //info.anisotropyEnable = true;
    info.maxAnisotropy = 16.0f;

    m_sampler = std::make_unique<vk::Sampler>(m_engine->getGraphics().device(), info);
}

void SkyboxManager::createDescriptorSetLayout() {
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

void SkyboxManager::createDescriptorPool() {
    vk::DescriptorPoolCreateInfo info = {};
    info.maxSets = 1;
    info.poolSizes = {
        { vk::DescriptorType::Sampler, 1 },
        { vk::DescriptorType::SampledImage, 1 }
    };

    m_descriptorPool = std::make_unique<vk::DescriptorPool>(m_engine->getGraphics().device(), info);
}

void SkyboxManager::createDecriptorSet() {
    vk::DescriptorSetAllocateInfo info = {};
    info.descriptorPool = m_descriptorPool.get();
    info.setLayouts = { *m_descriptorSetLayout };

    m_descriptorSet = std::make_unique<vk::DescriptorSet>(std::move(m_descriptorPool->allocate(info)[0]));
}

void SkyboxManager::writeDescriptorSet() {
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

void SkyboxManager::createPipelineLayout() {
    vk::PipelineLayoutCreateInfo info = {};
    info.setLayouts = {
        m_cameraSystem->descriptorLayout(),
        *m_descriptorSetLayout
    };

    m_pipelineLayout = std::make_unique<vk::PipelineLayout>(m_engine->getGraphics().device(), info);
}

static vk::ShaderModule createShaderModule(vk::Device& device, const std::vector<char>& byteCode) {
    vk::ShaderModuleCreateInfo info = {};
    info.code = byteCode;

    return vk::ShaderModule(device, info);
}

void SkyboxManager::createPipeline(vk::RenderPass& renderPass) {
    std::vector<char> vertShaderCode = readFile("shaders/skybox.vert.spv");
    std::vector<char> fragShaderCode = readFile("shaders/skybox.frag.spv");

    vk::ShaderModule vertShader = createShaderModule(m_engine->getGraphics().device(), vertShaderCode);
    vk::ShaderModule fragShader = createShaderModule(m_engine->getGraphics().device(), fragShaderCode);

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