#pragma once
#include <entt/entt.hpp>
#include <VulkanWrapper/VulkanWrapper.h>

namespace VoxelEngine {
    namespace UI {
        class Canvas;

        class Renderer {
        public:
            virtual void render(vk::CommandBuffer& commandBuffer, Canvas& canvas, entt::entity) = 0;
        };
    }
}