#pragma once
#include <VulkanWrapper/VulkanWrapper.h>
#include <Engine/Engine.h>
#include <Engine/RenderGraph/TransferNode.h>
#include "MipmapGenerator.h"

class TextureManager {
public:
    TextureManager(VoxelEngine::Engine& engine);

    const vk::DescriptorSetLayout& descriptorSetLayout() const { return *m_descriptorSetLayout; }
    const vk::DescriptorSet& descriptorSet() const { return *m_descriptorSet; }
    std::shared_ptr<VoxelEngine::Image> image() const { return m_image; }
    uint32_t count() const;
    uint32_t mipLevelCount() const;

    void createTexture(VoxelEngine::TransferNode& transferNode, MipmapGenerator& mipmapGenerator);

private:
    VoxelEngine::Engine* m_engine;
    std::shared_ptr<VoxelEngine::Image> m_image;
    std::unique_ptr<vk::ImageView> m_imageView;
    std::unique_ptr<vk::Sampler> m_sampler;
    std::unique_ptr<vk::DescriptorSetLayout> m_descriptorSetLayout;
    std::unique_ptr<vk::DescriptorPool> m_descriptorPool;
    std::unique_ptr<vk::DescriptorSet> m_descriptorSet;

    void createImage();
    void createImageView();
    void createSampler();
    void createDescriptorSetLayout();
    void createDescriptorPool();
    void createDescriptorSet();
    void writeDescriptorSet();
};