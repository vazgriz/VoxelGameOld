#pragma once
#include <VulkanWrapper/VulkanWrapper.h>
#include <Engine/System.h>
#include <Engine/RenderSystem.h>

class TriangleRenderer :public System {
public:
    TriangleRenderer(uint32_t priority, Renderer& renderer, RenderSystem& renderSystem);

    void update(Clock& clock);

private:
    Renderer* m_renderer;
    RenderSystem* m_renderSystem;
    std::unique_ptr<vk::CommandPool> m_commandPool;
    std::vector<vk::CommandBuffer> m_commandBuffers;
    std::unique_ptr<vk::RenderPass> m_renderPass;
    std::vector<vk::Framebuffer> m_framebuffers;

    void createRenderPass();
    void createFramebuffers();
};