#pragma once
#include <VulkanWrapper/VulkanWrapper.h>
#include "Engine/Window.h"
#include "Engine/MemoryManager.h"
#include <entt/signal/sigh.hpp>

namespace VoxelEngine {
    class Graphics {
    public:
        Graphics();

        vk::Instance& instance() const { return *m_instance; }
        vk::Surface& surface() const { return*m_surface; }
        vk::Device& device() const { return *m_device; }
        const vk::Queue* graphicsQueue() const { return m_graphicsQueue; }
        const vk::Queue* presentQueue() const { return m_presentQueue; }
        const vk::Queue* transferQueue() const { return m_transferQueue; }
        vk::Swapchain& swapchain() const { return *m_swapchain; }
        const std::vector<vk::ImageView>& swapchainImageViews() const { return m_swapchainImageViews; }
        MemoryManager& memory() const { return *m_memory; }

        entt::sink<void(vk::Swapchain&)>& onSwapchainChanged() { return m_onSwapchainChanged; }

        void pickPhysicalDevice(Window& window);

    private:
        Window* m_window;

        std::unique_ptr<vk::Instance> m_instance;
        std::unique_ptr<vk::Surface> m_surface;
        std::unique_ptr<vk::Device> m_device;

        std::unique_ptr<MemoryManager> m_memory;
        const vk::Queue* m_graphicsQueue;
        const vk::Queue* m_presentQueue;
        const vk::Queue* m_transferQueue;
        std::unique_ptr<vk::Swapchain> m_swapchain;
        std::vector<vk::ImageView> m_swapchainImageViews;

        entt::sigh<void(vk::Swapchain&)> m_onSwapchainChangedSignal;
        entt::sink<void(vk::Swapchain&)> m_onSwapchainChanged;

        void createInstance();
        void createSurface(Window& window);
        void createDevice(const vk::PhysicalDevice& physicalDevice, uint32_t graphicsIndex, uint32_t presentIndex, uint32_t transferIndex);
        void createSwapchain();
        void createImageViews();

        void recreateSwapchain(uint32_t width, uint32_t height);
    };
}