#include "UIManager.h"
#include <Engine/UI/Transform.h>
#include <Engine/UI/Panel.h>

UIManager::UIManager(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& renderGraph, VoxelEngine::Window& window) {
    m_engine = &engine;
    m_window = &window;
    m_renderGraph = &renderGraph;
}

void UIManager::setNode(VoxelEngine::UI::UINode& uiNode, VoxelEngine::TransferNode& transferNode) {
    m_node = &uiNode;
    m_panelRenderer = std::make_unique<VoxelEngine::UI::PanelRenderer>(*m_engine, uiNode);

    m_canvas = std::make_unique<VoxelEngine::UI::Canvas>(*m_engine, transferNode, m_window->getFramebufferWidth(), m_window->getFramebufferHeight());

    uiNode.addCanvas(*m_canvas);
    m_window->onFramebufferResized().connect<&VoxelEngine::UI::Canvas::setSize>(m_canvas.get());

    init();
}

void UIManager::init() {
    auto entity = m_canvas->createNode();
    auto& transform = m_canvas->registry().assign<VoxelEngine::UI::Transform>(entity, m_canvas->registry(), entity, entt::null);
    auto& element = m_canvas->registry().assign<std::unique_ptr<VoxelEngine::UI::Element>>(entity, std::make_unique<VoxelEngine::UI::Panel>(m_canvas->registry(), entity, m_panelRenderer.get()));
    auto& panel = *dynamic_cast<VoxelEngine::UI::Panel*>(element.get());

    transform.setPosition({ 10, 10, 0 });
    transform.setSize({ 20, 20 });
    panel.setColor({ 1.0, 0, 0, 1.0 });

    m_canvas->addRoot(entity);
}