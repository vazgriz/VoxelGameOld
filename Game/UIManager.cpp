#include "UIManager.h"
#include <Engine/UI/Transform.h>
#include <Engine/UI/Panel.h>

UIManager::UIManager(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& renderGraph, VoxelEngine::Window& window) {
    m_engine = &engine;
    m_window = &window;
    m_renderGraph = &renderGraph;

    m_canvas = std::make_unique<VoxelEngine::UI::Canvas>(engine, window.getFramebufferWidth(), window.getFramebufferHeight());

    m_panelRenderer = std::make_unique<VoxelEngine::UI::PanelRenderer>();

    window.onFramebufferResized().connect<&VoxelEngine::UI::Canvas::setSize>(m_canvas.get());

    init();
}

void UIManager::setNode(VoxelEngine::UI::UINode& node) {
    m_node = &node;
}

void UIManager::init() {
    auto entity = m_canvas->createNode();
    auto& transform = m_canvas->registry().assign<VoxelEngine::UI::Transform>(entity, m_canvas->registry(), entity, entt::null);
    auto& panel = m_canvas->registry().assign<VoxelEngine::UI::Panel>(entity, m_canvas->registry(), entity, m_panelRenderer.get());

    transform.setPosition({ 10, 10, 0 });
    transform.setSize({ 20, 20 });
    panel.setColor({ 1.0, 0, 0, 0 });

    m_canvas->addRoot(entity);
}