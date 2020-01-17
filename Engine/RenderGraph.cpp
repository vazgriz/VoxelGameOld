#include "Engine/RenderGraph/RenderGraph.h"
#include "Engine/DirectedAcyclicGraph.h"

using namespace VoxelEngine;

RenderGraph::BufferInput::BufferInput(Node& node, vk::AccessFlags accessMask, vk::PipelineStageFlags stages) {
    m_node = &node;
    m_dstFlags = accessMask;
    m_dstStages = stages;
}

void RenderGraph::BufferInput::input(const Buffer& buffer) {
    m_inputs.insert({ &buffer, { &buffer.buffer(), VK_WHOLE_SIZE, 0 } });
}

void RenderGraph::BufferInput::clear() {
    m_inputs.clear();
}

RenderGraph::BufferOutput::BufferOutput(Node& node, vk::AccessFlags accessMask, vk::PipelineStageFlags stages) {
    m_node = &node;
    m_srcFlags = accessMask;
    m_srcStages = stages;
}

void RenderGraph::BufferOutput::output(const Buffer& buffer) {
    m_outputs.insert({ &buffer, { &buffer.buffer(), VK_WHOLE_SIZE, 0 } });
}

void RenderGraph::BufferOutput::clear() {
    m_outputs.clear();
}

RenderGraph::ImageInput::ImageInput(Node& node, vk::AccessFlags accessMask, vk::PipelineStageFlags stages) {
    m_node = &node;
    m_dstFlags = accessMask;
    m_dstStages = stages;
}

void RenderGraph::ImageInput::clear() {

}

RenderGraph::ImageOutput::ImageOutput(Node& node, vk::AccessFlags accessMask, vk::PipelineStageFlags stages) {
    m_node = &node;
    m_srcFlags = accessMask;
    m_srcStages = stages;
}

void RenderGraph::ImageOutput::clear() {

}

RenderGraph::Edge::Edge(Node& source, Node& dest) {
    m_sourceNode = &source;
    m_destNode = &dest;
}

RenderGraph::BufferEdge::BufferEdge(RenderGraph::BufferOutput& source, RenderGraph::BufferInput& dest) 
: Edge(source.node(), dest.node()) {
    m_source = &source;
    m_dest = &dest;
}

void RenderGraph::BufferEdge::makeTransferEdge() {
    source().addOutputEdge(*this);
    dest().addInputEdge(*this);
}

RenderGraph::ImageEdge::ImageEdge(RenderGraph::ImageOutput& source, RenderGraph::ImageInput& dest)
    : Edge(source.node(), dest.node()) {
    m_source = &source;
    m_dest = &dest;
}

void RenderGraph::ImageEdge::makeTransferEdge() {
    source().addOutputEdge(*this);
    dest().addInputEdge(*this);
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

void RenderGraph::Node::addBufferInput(BufferInput& input) {
    m_bufferInputs.push_back(&input);
}

void RenderGraph::Node::addBufferOutput(BufferOutput& output) {
    m_bufferOutputs.push_back(&output);
}

void RenderGraph::Node::addImageInput(ImageInput& input) {
    m_imageInputs.push_back(&input);
}

void RenderGraph::Node::addImageOutput(ImageOutput& output) {
    m_imageOutputs.push_back(&output);
}

void RenderGraph::Node::addInputEdge(BufferEdge& edge) {
    m_inputBufferEdges.push_back(&edge);
}

void RenderGraph::Node::addOutputEdge(BufferEdge& edge) {
    m_outputBufferEdges.push_back(&edge);
}

void RenderGraph::Node::addInputEdge(ImageEdge& edge) {
    m_inputImageEdges.push_back(&edge);
}

void RenderGraph::Node::addOutputEdge(ImageEdge& edge) {
    m_outputImageEdges.push_back(&edge);
}

void RenderGraph::Node::addOutput(Node& output) {
    if (m_outputSet.insert(&output).second) {
        m_outputNodes.push_back(&output);
    }
}

void RenderGraph::Node::makeInputTransfers(vk::CommandBuffer& commandBuffer) {
    vk::PipelineStageFlags srcStages = {};
    vk::PipelineStageFlags dstStages = {};

    std::vector<vk::BufferMemoryBarrier> bufferBarriers;

    for (auto edge : m_inputBufferEdges) {
        for (auto& pair : edge->destInput().inputs()) {
            auto& outputs = edge->sourceOutput().outputs();
            auto it = outputs.find(pair.first);

            if (it != outputs.end()) {
                srcStages |= edge->sourceOutput().stages();
                dstStages |= edge->destInput().stages();

                vk::BufferMemoryBarrier barrier = {};
                barrier.buffer = &pair.first->buffer();
                barrier.size = pair.second.size;
                barrier.offset = pair.second.offset;
                barrier.srcAccessMask = edge->sourceOutput().flags();
                barrier.srcQueueFamilyIndex = edge->source().queue().familyIndex();
                barrier.dstAccessMask = edge->destInput().flags();
                barrier.dstQueueFamilyIndex = edge->dest().queue().familyIndex();

                bufferBarriers.push_back(barrier);
            }
        }
    }

    if (bufferBarriers.size() > 0) {
        commandBuffer.pipelineBarrier(
            srcStages, dstStages, vk::DependencyFlags::None,
            nullptr,
            bufferBarriers,
            nullptr
        );
    }
}

void RenderGraph::Node::makeOutputTransfers(vk::CommandBuffer& commandBuffer) {
    vk::PipelineStageFlags srcStages = {};
    vk::PipelineStageFlags dstStages = {};

    std::vector<vk::BufferMemoryBarrier> bufferBarriers;

    for (auto edge : m_outputBufferEdges) {
        for (auto& pair : edge->sourceOutput().outputs()) {
            auto& inputs = edge->destInput().inputs();
            auto it = inputs.find(pair.first);

            if (it != inputs.end()) {
                srcStages |= edge->sourceOutput().stages();
                dstStages |= edge->destInput().stages();

                vk::BufferMemoryBarrier barrier = {};
                barrier.buffer = &pair.first->buffer();
                barrier.size = pair.second.size;
                barrier.offset = pair.second.offset;
                barrier.srcAccessMask = edge->sourceOutput().flags();
                barrier.srcQueueFamilyIndex = edge->source().queue().familyIndex();
                barrier.dstAccessMask = edge->destInput().flags();
                barrier.dstQueueFamilyIndex = edge->dest().queue().familyIndex();

                bufferBarriers.push_back(barrier);
            }
        }
    }

    if (bufferBarriers.size() > 0) {
        commandBuffer.pipelineBarrier(
            srcStages, dstStages, vk::DependencyFlags::None,
            nullptr,
            bufferBarriers,
            nullptr
        );
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

    //makeInputTransfers(commandBuffer);
    render(currentFrame, commandBuffer);
    //makeOutputTransfers(commandBuffer);

    commandBuffer.end();
}

void RenderGraph::Node::submit(uint32_t currentFrame) {
    m_submitInfo.commandBuffers[0] = m_commandBuffers[currentFrame];

    m_queue->submit(m_submitInfo, &m_fences[currentFrame]);
}

void RenderGraph::Node::wait() {
    vk::Fence::wait(m_queue->device(), m_fences, true, -1);
}

void RenderGraph::Node::clearInputsAndOutputs() {
    for (auto& input : m_bufferInputs) {
        input->clear();
    }

    for (auto& output : m_bufferOutputs) {
        output->clear();
    }
    for (auto& input : m_imageInputs) {
        input->clear();
    }

    for (auto& output : m_imageOutputs) {
        output->clear();
    }
}

RenderGraph::RenderGraph(vk::Device& device, uint32_t framesInFlight) {
    m_device = &device;
    m_framesInFlight = framesInFlight;
    m_currentFrame = 0;
}

void RenderGraph::addEdge(std::unique_ptr<Edge> edge) {
    m_edges.emplace_back(std::move(edge));
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

    for (auto& edge : m_edges) {
        //if (edge->source().m_queue->familyIndex() != edge->dest().m_queue->familyIndex()) {
            m_transferEdges.push_back(edge.get());
            edge->makeTransferEdge();
        //}
    }
}

void RenderGraph::makeSemaphores() {
    for (Node* node : m_nodeList) {
        for (Node* outputNode : node->m_outputNodes) {
            vk::SemaphoreCreateInfo info = {};
            m_semaphores.emplace_back(std::make_unique<vk::Semaphore>(*m_device, info));
            auto& semaphore = *m_semaphores.back();

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
    for (auto node : m_nodeList) {
        node->preRender(m_currentFrame);
    }

    for (auto node : m_nodeList) {
        node->internalRender(m_currentFrame);
    }

    for (auto node : m_nodeList) {
        node->submit(m_currentFrame);
        node->clearInputsAndOutputs();
    }

    for (auto node : m_nodeList) {
        node->postRender(m_currentFrame);
    }

    m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;
}