#include <Engine/UI/PanelRenderer.h>
#include <glm/glm.hpp>
#include "Engine/Engine.h"
#include "Engine/Utilities.h"
#include "Engine/UI/UINode.h"
#include "Engine/UI/Canvas.h"

using namespace VoxelEngine;
using namespace VoxelEngine::UI;

PanelRenderer::PanelRenderer(Engine& engine, UINode& uiNode) {
    m_engine = &engine;
    m_graphics = &engine.getGraphics();
    m_uiNode = &uiNode;

    createPipelineLayout();
    createPipeline();
}

void PanelRenderer::render(vk::CommandBuffer& commandBuffer, Canvas& canvas, entt::entity entity) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::Graphics, *m_pipeline);

    vk::Viewport viewport = {};
    viewport.width = static_cast<float>(m_graphics->swapchain().extent().width);
    viewport.height = static_cast<float>(m_graphics->swapchain().extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    vk::Rect2D scissor = {};
    scissor.extent = m_graphics->swapchain().extent();

    commandBuffer.setViewport(0, { viewport });
    commandBuffer.setScissor(0, { scissor });

    glm::vec4 data = {};

    commandBuffer.pushConstants(*m_pipelineLayout, vk::ShaderStageFlags::Vertex, 0, sizeof(glm::vec4), &data);
    commandBuffer.draw(3, 1, 0, 0);
}


void PanelRenderer::createPipelineLayout() {
    vk::PipelineLayoutCreateInfo info = {};
    info.pushConstantRanges = {
        {
            vk::ShaderStageFlags::Vertex,
            0,
            sizeof(glm::ivec4)
        }
    };

    m_pipelineLayout = std::make_unique<vk::PipelineLayout>(m_graphics->device(), info);
}

void PanelRenderer::createPipeline() {
    std::vector<char> vertShaderCode = VoxelEngine::readFile("shaders/ui_panel.vert.spv");
    std::vector<char> fragShaderCode = VoxelEngine::readFile("shaders/ui_panel.frag.spv");

    vk::ShaderModule vertShader = VoxelEngine::createShaderModule(m_graphics->device(), vertShaderCode);
    vk::ShaderModule fragShader = VoxelEngine::createShaderModule(m_graphics->device(), fragShaderCode);

    vk::PipelineShaderStageCreateInfo vertInfo = {};
    vertInfo.module = &vertShader;
    vertInfo.name = "main";
    vertInfo.stage = vk::ShaderStageFlags::Vertex;

    vk::PipelineShaderStageCreateInfo fragInfo = {};
    fragInfo.module = &fragShader;
    fragInfo.name = "main";
    fragInfo.stage = vk::ShaderStageFlags::Fragment;

    std::vector<vk::PipelineShaderStageCreateInfo> stages = { std::move(vertInfo), std::move(fragInfo) };

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
    inputAssemblyInfo.topology = vk::PrimitiveTopology::TriangleList;

    vk::PipelineViewportStateCreateInfo viewportInfo = {};
    viewportInfo.viewports = { {} };    //still need 1 viewport and scissor here, even though they are dynamic
    viewportInfo.scissors = { {} };

    vk::PipelineRasterizationStateCreateInfo rasterizerInfo = {};
    rasterizerInfo.polygonMode = vk::PolygonMode::Fill;
    rasterizerInfo.lineWidth = 1.0f;
    rasterizerInfo.cullMode = vk::CullModeFlags::Back;
    rasterizerInfo.frontFace = vk::FrontFace::Clockwise;

    vk::PipelineMultisampleStateCreateInfo multisampleInfo = {};
    multisampleInfo.rasterizationSamples = vk::SampleCountFlags::_1;
    multisampleInfo.minSampleShading = 1.0f;

    vk::PipelineColorBlendAttachmentState colorBlendState = {};
    colorBlendState.colorWriteMask =
        vk::ColorComponentFlags::R
        | vk::ColorComponentFlags::G
        | vk::ColorComponentFlags::B
        | vk::ColorComponentFlags::A;

    vk::PipelineColorBlendStateCreateInfo colorBlendInfo = {};
    colorBlendInfo.attachments = { colorBlendState };

    vk::PipelineDynamicStateCreateInfo dynamicInfo = {};
    dynamicInfo.dynamicStates = { vk::DynamicState::Viewport, vk::DynamicState::Scissor };

    vk::GraphicsPipelineCreateInfo info = {};
    info.stages = stages;
    info.vertexInputState = &vertexInputInfo;
    info.inputAssemblyState = &inputAssemblyInfo;
    info.viewportState = &viewportInfo;
    info.rasterizationState = &rasterizerInfo;
    info.multisampleState = &multisampleInfo;
    info.colorBlendState = &colorBlendInfo;
    info.dynamicState = &dynamicInfo;
    info.layout = m_pipelineLayout.get();
    info.renderPass = &m_uiNode->renderPass();
    info.subpass = 0;

    m_pipeline = std::make_unique<vk::GraphicsPipeline>(m_graphics->device(), info);
}