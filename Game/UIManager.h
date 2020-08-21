#pragma once
#include <Engine/Engine.h>
#include <Engine/UI/Canvas.h>
#include <Engine/UI/UINode.h>
#include <Engine/UI/PanelRenderer.h>

class UIManager {
public:
    UIManager(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& renderGraph, VoxelEngine::Window& window);

    void setNode(VoxelEngine::UI::UINode& node);

private:
    VoxelEngine::Engine* m_engine;
    VoxelEngine::Window* m_window;
    VoxelEngine::RenderGraph* m_renderGraph;
    VoxelEngine::UI::UINode* m_node;

    std::unique_ptr<VoxelEngine::UI::Canvas> m_canvas;
    std::unique_ptr<VoxelEngine::UI::PanelRenderer> m_panelRenderer;

    void init();
};