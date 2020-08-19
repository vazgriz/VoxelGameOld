#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <entt/entt.hpp>

namespace VoxelEngine {
    namespace UI {
        class Transform {
        public:
            Transform(entt::registry& registry, entt::entity id, entt::entity parent);

            Transform(const Transform& other) = delete;
            Transform& operator = (const Transform& other) = delete;
            Transform(Transform&& other) = default;
            Transform& operator = ( Transform&& other) = default;

            entt::entity parent() const { return m_parent; }

            glm::vec3 position() const { return m_position; }
            glm::vec2 size() const { return m_size; }

            void setParent(entt::entity parent);

            void setPosition(glm::vec3 position);
            void setSize(glm::vec2 size);

        private:
            glm::vec3 m_position;
            glm::vec2 m_size;

            entt::registry* m_registry;
            entt::entity m_id;
            entt::entity m_parent;
            std::vector<entt::entity> m_children;
        };
    }
}