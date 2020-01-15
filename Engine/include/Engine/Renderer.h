#pragma once
#include <VulkanWrapper/VulkanWrapper.h>
#include "Engine/Window.h"

class Renderer {
public:
    Renderer();

    vk::Instance& instance() const { return *m_instance; }
    vk::Surface& surface() const { return*m_surface; }
    vk::Device& device() const { return *m_device; }
    const vk::Queue* graphicsQueue() const { return m_graphicsQueue; }
    const vk::Queue* presentQueue() const { return m_presentQueue; }
    vk::Swapchain& swapchain() const { return *m_swapchain; }
    const std::vector<vk::ImageView>& swapchainImageViews() const { return m_swapchainImageViews; }

    void pickPhysicalDevice(Window& window);

private:
    Window* m_window;

    std::unique_ptr<vk::Instance> m_instance;
    std::unique_ptr<vk::Surface> m_surface;
    std::unique_ptr<vk::Device> m_device;
    const vk::Queue* m_graphicsQueue;
    const vk::Queue* m_presentQueue;
    std::unique_ptr<vk::Swapchain> m_swapchain;
    std::vector<vk::ImageView> m_swapchainImageViews;

    void createInstance();
    void createSurface(Window& window);
    void createDevice(const vk::PhysicalDevice& physicalDevice, uint32_t graphicsIndex, uint32_t presentIndex);
    void createSwapchain();
    void createImageViews();
};