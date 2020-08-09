#include "TextureManager.h"
#include <stb_image.h>

std::vector<std::string> textureNames = {
    "resources/dirt.png",
    "resources/grass_side.png",
    "resources/grass_top.png",
    "resources/stone.png",
};

const uint32_t textureSize = 16;
const uint32_t mipLevels = 5;

TextureManager::TextureManager(VoxelEngine::Engine& engine) {
    m_engine = &engine;

    createImage();
    createImageView();
    createSampler();
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSet();
    writeDescriptorSet();
}

uint32_t TextureManager::count() const {
    return static_cast<uint32_t>(textureNames.size());
}

uint32_t TextureManager::mipLevelCount() const {
    return mipLevels;
}

void TextureManager::createTexture(VoxelEngine::TransferNode& transferNode, MipmapGenerator& mipmapGenerator) {
    for (uint32_t i = 0; i < textureNames.size(); i++) {
        auto& fileName = textureNames[i];
        int width;
        int height;
        int channels;

        auto data = stbi_load(fileName.c_str(), &width, &height, &channels, 4);

        vk::ImageSubresourceLayers subresource = {};
        subresource.aspectMask = vk::ImageAspectFlags::Color;
        subresource.baseArrayLayer = i;
        subresource.layerCount = 1;
        subresource.mipLevel = 0;

        transferNode.transfer(*m_image, {}, { textureSize, textureSize, 1 }, subresource, data);

        stbi_image_free(data);
    }

    mipmapGenerator.generate(m_image);
}

void TextureManager::createImage() {
    vk::ImageCreateInfo info = {};
    info.usage = vk::ImageUsageFlags::TransferSrc | vk::ImageUsageFlags::TransferDst | vk::ImageUsageFlags::Sampled;
    info.format = vk::Format::R8G8B8A8_Unorm;
    info.arrayLayers = static_cast<uint32_t>(textureNames.size());
    info.extent = { textureSize, textureSize, 1 };
    info.imageType = vk::ImageType::_2D;
    info.mipLevels = mipLevels;
    info.samples = vk::SampleCountFlags::_1;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_image = std::make_shared<VoxelEngine::Image>(*m_engine, info, allocInfo);
}

void TextureManager::createImageView() {
    vk::ImageViewCreateInfo info = {};
    info.image = &m_image->image();
    info.format = vk::Format::R8G8B8A8_Unorm;
    info.viewType = vk::ImageViewType::_2D_Array;
    info.subresourceRange.aspectMask = vk::ImageAspectFlags::Color;
    info.subresourceRange.layerCount = static_cast<uint32_t>(textureNames.size());
    info.subresourceRange.levelCount = mipLevels;

    m_imageView = std::make_unique<vk::ImageView>(m_engine->getGraphics().device(), info);
}

void TextureManager::createSampler() {
    vk::SamplerCreateInfo info = {};
    info.magFilter = vk::Filter::Nearest;
    info.minFilter = vk::Filter::Nearest;
    info.mipmapMode = vk::SamplerMipmapMode::Linear;
    info.addressModeU = vk::SamplerAddressMode::ClampToEdge;
    info.addressModeV = vk::SamplerAddressMode::ClampToEdge;
    info.anisotropyEnable = true;
    info.maxAnisotropy = 16.0f;
    info.minLod = 0;
    info.maxLod = static_cast<float>(mipLevels);

    m_sampler = std::make_unique<vk::Sampler>(m_engine->getGraphics().device(), info);
}

void TextureManager::createDescriptorSetLayout() {
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

void TextureManager::createDescriptorPool() {
    vk::DescriptorPoolCreateInfo info = {};
    info.maxSets = 1;
    info.poolSizes = {
        { vk::DescriptorType::Sampler, 1 },
        { vk::DescriptorType::SampledImage, 1 }
    };

    m_descriptorPool = std::make_unique<vk::DescriptorPool>(m_engine->getGraphics().device(), info);
}

void TextureManager::createDescriptorSet() {
    vk::DescriptorSetAllocateInfo info = {};
    info.descriptorPool = m_descriptorPool.get();
    info.setLayouts = { *m_descriptorSetLayout };

    m_descriptorSet = std::make_unique<vk::DescriptorSet>(std::move(m_descriptorPool->allocate(info)[0]));
}

void TextureManager::writeDescriptorSet() {
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