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
    std::unique_ptr<vk::PipelineLayout> m_pipelineLayout;
    std::unique_ptr<vk::Pipeline> m_pipeline;

    void createRenderPass();
    void createFramebuffers();
    void createPipelineLayout();
    void createPipeline();
};