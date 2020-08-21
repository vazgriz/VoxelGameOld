#pragma once
#include <entt/entt.hpp>
#include <VulkanWrapper/VulkanWrapper.h>

namespace VoxelEngine {
    namespace UI {
        class Canvas;

        class Element {
        public:
            Element(entt::registry& registry, entt::entity id);
            Element(const Element& other) = delete;
            Element& operator = (const Element& other) = delete;
            Element (Element&& other) = default;
            Element& operator = (Element&& other) = default;

            entt::registry& registry() const { return *m_registry; }
            entt::entity id() const { return m_id; }

            virtual void render(Canvas& canvas, vk::CommandBuffer& commandBuffer) = 0;

        protected:
            entt::registry* m_registry;
            entt::entity m_id;
        };

        using ElementUPtr = std::unique_ptr<Element>;
    }
}