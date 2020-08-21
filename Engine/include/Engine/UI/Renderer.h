#pragma once
#include <entt/entt.hpp>
#include <VulkanWrapper/VulkanWrapper.h>

namespace VoxelEngine {
    namespace UI {
        class Renderer {
        public:
            virtual void render(entt::entity entity, vk::CommandBuffer& commandBuffer) = 0;
        };
    }
}