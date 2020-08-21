#pragma once
#include <Engine/UI/Element.h>
#include <glm/glm.hpp>
#include <entt/entt.hpp>

namespace VoxelEngine {
    namespace UI {
        class PanelRenderer;

        class Panel : public Element {
        public:
            Panel(entt::registry& registry, entt::entity id, PanelRenderer* renderer);
            void render(Canvas& canvas, vk::CommandBuffer& commandBuffer);

            glm::vec4 color() { return m_color; }
            void setColor(glm::vec4 color) { m_color = color; }

        private:
            PanelRenderer* m_renderer;
            glm::vec4 m_color;
        };
    }
}