#pragma once
#include <Engine/UI/Renderer.h>
#include <VulkanWrapper/VulkanWrapper.h>

namespace VoxelEngine {
    class Engine;
    class Graphics;

    namespace UI {
        class UINode;
        class Canvas;

        class PanelRenderer : public Renderer {
        public:
            PanelRenderer(Engine& engine, UINode& uiNode);
            PanelRenderer(const PanelRenderer& other) = delete;
            PanelRenderer& operator = (const PanelRenderer& other) = delete;
            PanelRenderer(PanelRenderer&& other) = default;
            PanelRenderer& operator = (PanelRenderer&& other) = default;

            void render(vk::CommandBuffer& commandBuffer, Canvas& canvas, entt::entity entity);

        private:
            Engine* m_engine;
            Graphics* m_graphics;
            UINode* m_uiNode;

            std::unique_ptr<vk::PipelineLayout> m_pipelineLayout;
            std::unique_ptr<vk::Pipeline> m_pipeline;

            void createPipelineLayout();
            void createPipeline();
        };
    }
}