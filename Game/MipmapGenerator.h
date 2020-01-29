#pragma once
#include <Engine/RenderGraph/RenderGraph.h>

class MipmapGenerator : public VoxelEngine::RenderGraph::Node {
public:
    MipmapGenerator(VoxelEngine::Engine& engine, VoxelEngine::RenderGraph& graph);

    VoxelEngine::RenderGraph::ImageUsage& inputUsage() const { return *m_inputImageUsage; }
    VoxelEngine::RenderGraph::ImageUsage& outputUsage() const { return *m_outputImageUsage; }

    void generate(const std::shared_ptr<VoxelEngine::Image>& image);

    void preRender(uint32_t currentFrame);
    void render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer);
    void postRender(uint32_t currentFrame) {}

private:
    VoxelEngine::Engine* m_engine;
    std::unique_ptr<VoxelEngine::RenderGraph::ImageUsage> m_inputImageUsage;
    std::unique_ptr<VoxelEngine::RenderGraph::ImageUsage> m_outputImageUsage;
    std::vector<std::shared_ptr<VoxelEngine::Image>> m_images;
};