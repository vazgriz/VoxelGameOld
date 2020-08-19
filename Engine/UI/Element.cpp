#include <Engine/UI/Element.h>

using namespace VoxelEngine;
using namespace VoxelEngine::UI;

Element::Element(entt::registry& registry, entt::entity id) {
    m_registry = &registry;
    m_id = id;
}