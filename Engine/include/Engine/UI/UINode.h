#pragma once
#include "Engine/RenderGraph/RenderGraph.h"
#include <VulkanWrapper/VulkanWrapper.h>

namespace VoxelEngine {
    namespace UI {
        class Canvas;

        class UINode : public RenderGraph::Node {
        public:
            UINode(Engine& engine, RenderGraph& graph);

            UINode(const UINode& other) = delete;
            UINode& operator = (const UINode& other) = delete;
            UINode(UINode&& other) = default;
            UINode& operator = (UINode&& other) = default;

            void preRender(uint32_t currentFrame);
            void render(uint32_t currentFrame, vk::CommandBuffer & commandBuffer);
            void postRender(uint32_t currentFrame) {};

            RenderGraph::ImageUsage& imageUsage() const { return *m_imageUsage; }
            vk::RenderPass& renderPass() const { return *m_renderPass; }
            vk::DescriptorSetLayout& cameraDescriptorSetLayout() const { return *m_descriptorSetLayout; }

            void addCanvas(Canvas& canvas);
            Canvas& getCanvas(size_t index) const { return *m_canvases[index]; }

        private:
            Engine* m_engine;
            Graphics* m_graphics;
            RenderGraph* m_renderGraph;

            std::vector<Canvas*> m_canvases;

            std::unique_ptr<RenderGraph::ImageUsage> m_imageUsage;
            std::unique_ptr<vk::RenderPass> m_renderPass;
            std::unique_ptr<vk::DescriptorSetLayout> m_descriptorSetLayout;

            void createRenderPass();
            void createDescriptorSetLayout();
        };
    }
}