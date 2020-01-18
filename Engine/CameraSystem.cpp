#include "Engine/CameraSystem.h"
#include "Engine/UniformWriter.h"

using namespace VoxelEngine;

struct CameraUniform {
    glm::mat4 view;
    glm::mat4 projection;
};

CameraSystem::CameraSystem(Engine& engine, uint32_t priority) : System(priority) {
    m_engine = &engine;

    createDescriptorPool();
    createDescriptorSetLayout();
    createDescriptorSet();
    createUniformBuffer();
}

void CameraSystem::update(Clock& clock) {
    updateUniform();
}

void CameraSystem::updateUniform() {
    CameraUniform uniform = {};
    uniform.view = m_camera->viewMatrix();
    uniform.projection = m_camera->projectionMatrix();

    m_transferNode->transfer(*m_uniformBuffer, sizeof(CameraUniform), 0, &uniform);
}

void CameraSystem::createDescriptorPool() {
    vk::DescriptorPoolCreateInfo info = {};
    info.maxSets = 1;
    info.poolSizes = { { vk::DescriptorType::UniformBuffer, 1 } };

    m_descriptorPool = std::make_unique<vk::DescriptorPool>(m_engine->getGraphics().device(), info);
}

void CameraSystem::createDescriptorSetLayout() {
    vk::DescriptorSetLayoutBinding binding = {};
    binding.binding = 0;
    binding.descriptorType = vk::DescriptorType::UniformBuffer;
    binding.descriptorCount = 1;
    binding.stageFlags = vk::ShaderStageFlags::Vertex;

    vk::DescriptorSetLayoutCreateInfo info = {};
    info.bindings = { binding };

    m_descriptorSetLayout = std::make_unique<vk::DescriptorSetLayout>(m_engine->getGraphics().device(), info);
}

void CameraSystem::createDescriptorSet() {
    vk::DescriptorSetAllocateInfo info = {};
    info.descriptorPool = m_descriptorPool.get();
    info.setLayouts = { *m_descriptorSetLayout };

    m_descriptorSet = std::make_unique<vk::DescriptorSet>(std::move(m_descriptorPool->allocate(info)[0]));
}

void CameraSystem::createUniformBuffer() {
    vk::BufferCreateInfo info = {};
    info.size = sizeof(CameraUniform);
    info.usage = vk::BufferUsageFlags::UniformBuffer | vk::BufferUsageFlags::TransferDst;

    VmaAllocationCreateInfo allocInfo = {};
    allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
    allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    m_uniformBuffer = std::make_unique<Buffer>(*m_engine, info, allocInfo);
}