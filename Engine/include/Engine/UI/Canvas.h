#pragma once
#include <stdint.h>
#include <entt/entt.hpp>
#include <VulkanWrapper/VulkanWrapper.h>

namespace VoxelEngine {
    class Engine;
    class Graphics;
    class Image;
    class TransferNode;
    class Buffer;

    namespace UI {
        class Canvas {
        public:
            Canvas(Engine& engine, TransferNode& transferNode, uint32_t width, uint32_t height);
            Canvas(const Canvas& other) = delete;
            Canvas& operator = (const Canvas& other) = delete;
            Canvas(Canvas&& other) = default;
            Canvas& operator = (Canvas&& other) = default;

            entt::registry& registry() { return m_registry; }
            VoxelEngine::Image& image() const { return *m_image; }
            vk::ImageView& imageView() const { return *m_imageView; }
            vk::Framebuffer& framebuffer() const { return *m_framebuffer; }
            vk::DescriptorSet& cameraDescriptor() const { return *m_descriptorSet; }

            uint32_t height() { return m_height; }
            uint32_t width() { return m_width; }

            void setSize(uint32_t width, uint32_t height);
            void setRenderPass(vk::RenderPass& renderPass);
            void setCameraDescriptorLayout(vk::DescriptorSetLayout& layout);

            void addRoot(entt::entity entity);
            entt::entity createRoot();
            entt::entity createNode();

            void preRender();
            void render(vk::CommandBuffer& commandBuffer);

        private:
            Engine* m_engine;
            Graphics* m_graphics;
            TransferNode* m_transferNode;
            uint32_t m_width;
            uint32_t m_height;

            entt::registry m_registry;

            std::vector<entt::entity> m_roots;

            vk::RenderPass* m_renderPass;
            vk::DescriptorSetLayout* m_descriptorSetLayout;
            std::unique_ptr<VoxelEngine::Image> m_image;
            std::unique_ptr<vk::ImageView> m_imageView;
            std::unique_ptr<vk::Framebuffer> m_framebuffer;
            std::unique_ptr<vk::DescriptorPool> m_descriptorPool;
            std::unique_ptr<vk::DescriptorSet> m_descriptorSet;
            std::unique_ptr<VoxelEngine::Buffer> m_uniformBuffer;

            void updateResources();

            void createImage();
            void createImageView();
            void createFramebuffer();
            void createUniformBuffer();
            void createDescriptorPool();
            void createDescriptorSet();
            void writeDescriptor();
        };
    }
}