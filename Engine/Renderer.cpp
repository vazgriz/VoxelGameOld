#include "Engine/Renderer.h"
#include "Engine/Window.h"
#include <GLFW/glfw3.h>
#include <unordered_set>
#include <limits>
#include <algorithm>

const std::vector<std::string> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueIndices {
    uint32_t graphicsFamily = -1;
    uint32_t presentFamily = -1;

    bool valid() {
        return graphicsFamily != -1 && presentFamily != -1;
    }
};

Renderer::Renderer() {
    createInstance();
}

void Renderer::createInstance() {
    vk::ApplicationInfo appInfo = {};
    appInfo.applicationName = "Voxel Game";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineName = "Voxel Game";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    vk::InstanceCreateInfo info = {};
    info.applicationInfo = &appInfo;

    for (size_t i = 0; i < glfwExtensionCount; i++) {
        info.enabledExtensionNames.push_back(glfwExtensions[i]);
    }

    info.enabledLayerNames = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    m_instance = std::make_unique<vk::Instance>(info);
}

QueueIndices getIndices(const vk::PhysicalDevice& physicalDevice, const vk::Surface& surface) {
    QueueIndices indices;

    for (uint32_t i = 0; i < physicalDevice.queueFamilies().size(); i++) {
        auto& queueFamily = physicalDevice.queueFamilies()[i];

        if ((queueFamily.queueFlags & vk::QueueFlags::Graphics) == vk::QueueFlags::Graphics) {
            indices.graphicsFamily = i;
        }

        bool presentSupport = surface.supported(physicalDevice, i);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if (indices.valid()) {
            break;
        }
    }

    return indices;
}

void Renderer::pickPhysicalDevice(Window& window) {
    m_window = &window;

    if (m_instance->physicalDevices().size() == 0) {
        throw std::runtime_error("Failed to find physical device with Vulkan support");
    }

    createSurface(window);

    for (auto& physicalDevice : m_instance->physicalDevices()) {
        QueueIndices indices = getIndices(physicalDevice, *m_surface);

        if (indices.valid()) {
            createDevice(physicalDevice, indices.graphicsFamily, indices.presentFamily);
            break;
        }
    }

    createSwapchain();
}

void Renderer::createSurface(Window& window) {
    VkSurfaceKHR surface;
    VKW_CHECK(glfwCreateWindowSurface(m_instance->handle(), window.handle(), m_instance->callbacks(), &surface));
    m_surface = std::make_unique<vk::Surface>(*m_instance, surface);
}

void Renderer::createDevice(const vk::PhysicalDevice& physicalDevice, uint32_t graphicsIndex, uint32_t presentIndex) {
    std::vector<vk::DeviceQueueCreateInfo> queueInfos;
    std::unordered_set<uint32_t> queueFamilySet = { graphicsIndex, presentIndex };

    float priority = 1.0f;
    for (auto index : queueFamilySet) {
        vk::DeviceQueueCreateInfo queueInfo = {};
        queueInfo.queueFamilyIndex = index;
        queueInfo.queueCount = 1;
        queueInfo.queuePriorities = { priority };
        queueInfos.emplace_back(std::move(queueInfo));
    }

    vk::PhysicalDeviceFeatures features = {};

    vk::DeviceCreateInfo info = {};
    info.queueCreateInfos = std::move(queueInfos);
    info.enabledFeatures = &features;
    info.enabledExtensionNames = deviceExtensions;

    m_device = std::make_unique<vk::Device>(physicalDevice, info);

    m_graphicsQueue = &m_device->getQueue(graphicsIndex, 0);
    m_presentQueue = &m_device->getQueue(presentIndex, 0);
}

vk::SurfaceFormat chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormat>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == vk::Format::B8G8R8A8_Unorm && availableFormat.colorSpace == vk::ColorSpace::SrgbNonlinear) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentMode chooseSwapPresentMode(const std::vector<vk::PresentMode>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == vk::PresentMode::Immediate) {
            return availablePresentMode;
        }
    }

    return vk::PresentMode::Fifo;
}

vk::Extent2D chooseSwapExtent(const Window& window, const vk::SurfaceCapabilities& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = { window.getWidth(), window.getHeight() };

        actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

void Renderer::createSwapchain() {
    vk::SurfaceCapabilities capabilities = m_surface->getCapabilities(m_device->physicalDevice());
    std::vector<vk::SurfaceFormat> formats = m_surface->getFormats(m_device->physicalDevice());
    std::vector<vk::PresentMode> presentModes = m_surface->getPresentModes(m_device->physicalDevice());

    if (formats.size() == 0 || presentModes.size() == 0) {
        throw std::runtime_error("Failed to create swapchain");
    }

    vk::SurfaceFormat format = chooseSwapSurfaceFormat(formats);
    vk::PresentMode presentMode = chooseSwapPresentMode(presentModes);
    vk::Extent2D extent = chooseSwapExtent(*m_window, capabilities);

    uint32_t imageCount = std::min<uint32_t>(std::max<uint32_t>(capabilities.minImageCount, 2), capabilities.maxImageCount);

    vk::SwapchainCreateInfo info = {};
    info.surface = m_surface.get();
    info.minImageCount = imageCount;
    info.imageFormat = format.format;
    info.imageColorSpace = format.colorSpace;
    info.imageExtent = extent;
    info.presentMode = presentMode;
    info.imageUsage = vk::ImageUsageFlags::ColorAttachment;
    info.imageArrayLayers = 1;

    if (m_graphicsQueue->familyIndex() == m_presentQueue->familyIndex()) {
        info.imageSharingMode = vk::SharingMode::Exclusive;
    } else {
        info.imageSharingMode = vk::SharingMode::Concurrent;
        info.queueFamilyIndices = { m_graphicsQueue->familyIndex(), m_presentQueue->familyIndex() };
    }

    info.preTransform = vk::SurfaceTransformFlags::Identity;
    info.compositeAlpha = vk::CompositeAlphaFlags::Opaque;
    info.clipped = true;
    info.oldSwapchain = m_swapchain.get();

    m_swapchain = std::make_unique<vk::Swapchain>(*m_device, info);
}

void Renderer::createImageViews() {
    for (auto& image : m_swapchain->images()) {
        vk::ImageViewCreateInfo info = {};
        info.image = &image;
        info.viewType = vk::ImageViewType::_2D;
        info.format = m_swapchain->format();
        info.components.r = vk::ComponentSwizzle::Identity;
        info.components.g = vk::ComponentSwizzle::Identity;
        info.components.b = vk::ComponentSwizzle::Identity;
        info.components.a = vk::ComponentSwizzle::Identity;
        info.subresourceRange.aspectMask = vk::ImageAspectFlags::Color;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;

        m_swapchainImageViews.emplace_back(*m_device, info);
    }
}