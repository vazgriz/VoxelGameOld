#pragma once
#include <Engine/UI/Renderer.h>

namespace VoxelEngine {
    namespace UI {
        class PanelRenderer : public Renderer {
        public:
            PanelRenderer();
            PanelRenderer(const PanelRenderer& other) = delete;
            PanelRenderer& operator = (const PanelRenderer& other) = delete;
            PanelRenderer(PanelRenderer&& other) = default;
            PanelRenderer& operator = (PanelRenderer&& other) = default;

            void render(entt::entity entity, vk::CommandBuffer& commandBuffer);
        };
    }
}