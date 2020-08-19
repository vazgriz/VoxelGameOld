#pragma once
#include <stdint.h>
#include <entt/entt.hpp>
#include <VulkanWrapper/VulkanWrapper.h>

namespace VoxelEngine {
    class Engine;

    namespace UI {
        class Canvas {
        public:
            Canvas(Engine& engine, uint32_t width, uint32_t height);
            Canvas(const Canvas& other) = delete;
            Canvas& operator = (const Canvas& other) = delete;
            Canvas(Canvas&& other) = default;
            Canvas& operator = (Canvas&& other) = default;

            entt::registry& registry() { return m_registry; }
            vk::Image& image() const { return *m_image; }
            vk::ImageView& imageView() const { return *m_imageView; }
            vk::Framebuffer& framebuffer() const { return *m_framebuffer; }

            uint32_t height() { return m_height; }
            uint32_t width() { return m_width; }

            void setSize(uint32_t width, uint32_t height);

            void addRoot(entt::entity entity);
            entt::entity createRoot();
            entt::entity createNode();

        private:
            Engine* m_engine;
            vk::Device* m_device;
            uint32_t m_width;
            uint32_t m_height;

            entt::registry m_registry;

            std::vector<entt::entity> m_roots;

            std::unique_ptr<vk::Image> m_image;
            std::unique_ptr<vk::ImageView> m_imageView;
            std::unique_ptr<vk::Framebuffer> m_framebuffer;

            void createImage();
            void createImageView();
            void createFramebuffer();
        };
    }
}