#pragma once
#include "Engine/Engine.h"
#include "Engine/RenderGraph/TransferNode.h"
#include <VulkanWrapper/VulkanWrapper.h>
#include <glm/glm.hpp>
#include <glm/ext/quaternion_float.hpp>

namespace VoxelEngine {
    class CameraSystem : public VoxelEngine::System {
    public:
        CameraSystem(Engine& engine);

        const vk::DescriptorSetLayout& descriptorLayout() const { return *m_descriptorSetLayout; }
        const vk::DescriptorSet& descriptorSet() const { return *m_descriptorSet; }
        std::shared_ptr<Buffer> uniformBuffer() const { return m_uniformBuffer; }

        void setCamera(Camera& camera) { m_camera = &camera; }
        void setTransferNode(TransferNode& transferNode) { m_transferNode = &transferNode; }

        void update(VoxelEngine::Clock& clock);

    private:
        Engine* m_engine;
        TransferNode* m_transferNode;
        Camera* m_camera;

        std::unique_ptr<vk::DescriptorPool> m_descriptorPool;
        std::unique_ptr<vk::DescriptorSetLayout> m_descriptorSetLayout;
        std::unique_ptr<vk::DescriptorSet> m_descriptorSet;
        std::shared_ptr<VoxelEngine::Buffer> m_uniformBuffer;

        void createDescriptorPool();
        void createDescriptorSetLayout();
        void createDescriptorSet();
        void createUniformBuffer();
        void writeDescriptorSet();

        void updateUniform();
    };
}