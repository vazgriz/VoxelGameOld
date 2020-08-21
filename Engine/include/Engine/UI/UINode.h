#pragma once
#include "Engine/RenderGraph/RenderGraph.h"
#include <VulkanWrapper/VulkanWrapper.h>

namespace VoxelEngine {
    namespace UI {
        class UINode : public RenderGraph::Node {
        public:
            UINode(RenderGraph& graph, const vk::Queue& queue);

            UINode(const UINode& other) = delete;
            UINode& operator = (const UINode& other) = delete;
            UINode(UINode&& other) = default;
            UINode& operator = (UINode&& other) = default;

            void preRender(uint32_t currentFrame) {};
            void render(uint32_t currentFrame, vk::CommandBuffer & commandBuffer);
            void postRender(uint32_t currentFrame) {};

            RenderGraph::ImageUsage& imageUsage() const { return *m_imageUsage; }

        private:
            std::unique_ptr<RenderGraph::ImageUsage> m_imageUsage;
        };
    }
}