#include "Engine/RenderGraph/RenderGraph.h"
#include "Engine/DirectedAcyclicGraph.h"

using namespace VoxelEngine;

RenderGraph::BufferInput::BufferInput(Node& node, vk::AccessFlags accessMask) {
    m_node = &node;
    m_dstFlags = accessMask;
}

RenderGraph::BufferOutput::BufferOutput(Node& node, vk::AccessFlags accessMask) {
    m_node = &node;
    m_srcFlags = accessMask;
}

RenderGraph::ImageInput::ImageInput(Node& node, vk::AccessFlags accessMask) {
    m_node = &node;
    m_dstFlags = accessMask;
}

RenderGraph::ImageOutput::ImageOutput(Node& node, vk::AccessFlags accessMask) {
    m_node = &node;
    m_srcFlags = accessMask;
}

RenderGraph::Edge::Edge(Node& source, Node& dest) {
    m_sourceNode = &source;
    m_destNode = &dest;
}

RenderGraph::BufferEdge::BufferEdge(const RenderGraph::BufferOutput& source, const RenderGraph::BufferInput& dest) 
: Edge(source.node(), dest.node()) {
    m_source = &source;
    m_dest = &dest;
}

RenderGraph::ImageEdge::ImageEdge(const RenderGraph::ImageOutput& source, const RenderGraph::ImageInput& dest)
    : Edge(source.node(), dest.node()) {
    m_source = &source;
    m_dest = &dest;
}

RenderGraph::Node::Node(RenderGraph& graph, const vk::Queue& queue, vk::PipelineStageFlags stages) {
    m_graph = &graph;
    m_queue = &queue;
    m_stages = stages;

    vk::CommandPoolCreateInfo info = {};
    info.queueFamilyIndex = m_queue->familyIndex();
    info.flags = vk::CommandPoolCreateFlags::ResetCommandBuffer;

    m_commandPool = std::make_unique<vk::CommandPool>(m_queue->device(), info);

    vk::CommandBufferAllocateInfo allocInfo = {};
    allocInfo.commandPool = m_commandPool.get();
    allocInfo.commandBufferCount = graph.framesInFlight();
    allocInfo.level = vk::CommandBufferLevel::Primary;

    m_commandBuffers = m_commandPool->allocate(allocInfo);

    for (size_t i = 0; i < graph.framesInFlight(); i++) {
        vk::FenceCreateInfo info = {};
        info.flags = vk::FenceCreateFlags::Signaled;
        m_fences.emplace_back(graph.device(), info);
    }

    m_submitInfo = {};
    m_submitInfo.commandBuffers = { m_commandBuffers[0] };
}

void RenderGraph::Node::addExternalWait(vk::Semaphore& semaphore, vk::PipelineStageFlags stages) {
    m_submitInfo.waitSemaphores.push_back(semaphore);
    m_submitInfo.waitDstStageMask.push_back(stages);
}

void RenderGraph::Node::addExternalSignal(vk::Semaphore& semaphore) {
    m_submitInfo.signalSemaphores.push_back(semaphore);
}

void RenderGraph::Node::addBufferInput(const BufferInput& input) {
    m_bufferInputs.push_back(&input);
}

void RenderGraph::Node::addBufferOutput(const BufferOutput& output) {
    m_bufferOutputs.push_back(&output);
}

void RenderGraph::Node::addImageInput(const ImageInput& input) {
    m_imageInputs.push_back(&input);
}

void RenderGraph::Node::addImageOutput(const ImageOutput& output) {
    m_imageOutputs.push_back(&output);
}

void RenderGraph::Node::addOutput(Node& output) {
    if (m_outputSet.insert(&output).second) {
        m_outputNodes.push_back(&output);
    }
}

void RenderGraph::Node::internalRender(uint32_t currentFrame) {
    vk::Fence& fence = m_fences[currentFrame];
    fence.wait();
    fence.reset();

    vk::CommandBuffer& commandBuffer = m_commandBuffers[currentFrame];

    commandBuffer.reset(vk::CommandBufferResetFlags::None);

    vk::CommandBufferBeginInfo info = {};
    info.flags = vk::CommandBufferUsageFlags::OneTimeSubmit;

    commandBuffer.begin(info);

    render(currentFrame, commandBuffer);

    commandBuffer.end();
}

void RenderGraph::Node::submit(uint32_t currentFrame) {
    m_submitInfo.commandBuffers[0] = m_commandBuffers[currentFrame];

    m_queue->submit(m_submitInfo, &m_fences[currentFrame]);
}

void RenderGraph::Node::wait() {
    vk::Fence::wait(m_queue->device(), m_fences, true, -1);
}

RenderGraph::RenderGraph(vk::Device& device, uint32_t framesInFlight) {
    m_device = &device;
    m_framesInFlight = framesInFlight;
    m_currentFrame = 0;
}

void RenderGraph::addEdge(RenderGraph::Edge&& edge) {
    m_edges.emplace_back(std::make_unique<RenderGraph::Edge>(std::move(edge)));
    auto& edge_ = *m_edges.back();
    edge_.source().addOutput(edge_.dest());
}

void RenderGraph::bake() {
    std::unordered_set<Node*> nodeSet;

    for (auto& ptr : m_nodes) {
        nodeSet.insert(ptr.get());
    }

    m_nodeList = topologicalSort<Node>(nodeSet, [](Node* node) {
        return node->m_outputNodes;
    });

    makeSemaphores();
}

void RenderGraph::makeSemaphores() {
    for (Node* node : m_nodeList) {
        for (Node* outputNode : node->m_outputNodes) {
            vk::SemaphoreCreateInfo info = {};
            m_semaphores.emplace_back(*m_device, info);
            auto& semaphore = m_semaphores.back();

            node->m_submitInfo.signalSemaphores.push_back(semaphore);
            outputNode->m_submitInfo.waitSemaphores.push_back(semaphore);
            outputNode->m_submitInfo.waitDstStageMask.push_back(outputNode->m_stages);
        }
    }
}

void RenderGraph::wait() {
    for (Node* node : m_nodeList) {
        node->wait();
    }
}

void RenderGraph::execute() const {
    m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;

    for (auto node : m_nodeList) {
        node->preRender(m_currentFrame);
    }

    for (auto node : m_nodeList) {
        node->internalRender(m_currentFrame);
    }

    for (auto node : m_nodeList) {
        node->submit(m_currentFrame);
    }

    for (auto node : m_nodeList) {
        node->postRender(m_currentFrame);
    }
}