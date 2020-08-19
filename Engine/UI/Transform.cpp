#include <Engine/UI/Transform.h>

using namespace VoxelEngine;
using namespace VoxelEngine::UI;

Transform::Transform(entt::registry& registry, entt::entity id, entt::entity parent) {
    m_registry = &registry;
    m_id = id;
    m_parent = parent;
    m_position = {};
    m_size = {};
}

void Transform::setParent(entt::entity parent) {
    auto view = m_registry->view<Transform>();

    //remove this from current parent
    if (m_parent != entt::null) {
        auto& oldParent = view.get<Transform>(m_parent);
        oldParent.m_children.erase(std::remove(oldParent.m_children.begin(), oldParent.m_children.end(), m_id), oldParent.m_children.end());
    }

    m_parent = parent;

    if (m_parent != entt::null) {
        auto& newParent = view.get<Transform>(m_parent);
        newParent.m_children.push_back(m_id);
    }
}

void Transform::setPosition(glm::vec3 position) {
    m_position = position;
}

void Transform::setSize(glm::vec2 size) {
    m_size = size;
}