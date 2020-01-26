#include "Engine/RenderGraph/RenderGraph.h"
#include "Engine/DirectedAcyclicGraph.h"

using namespace VoxelEngine;

RenderGraph::BufferUsage::BufferUsage(Node& node, vk::AccessFlags accessMask, vk::PipelineStageFlags stageFlags) {
    m_node = &node;
    m_node->addUsage(*this);
    m_accessMask = accessMask;
    m_stageFlags = stageFlags;
    m_buffers.resize(node.graph().framesInFlight());
}

void RenderGraph::BufferUsage::sync(std::shared_ptr<Buffer> buffer, vk::DeviceSize size, vk::DeviceSize offset) {
    vk::Buffer* vkBuffer = &buffer->buffer();
    m_buffers[m_node->currentFrame()].insert({ vkBuffer, { buffer, vkBuffer, size, offset } });
}

void RenderGraph::BufferUsage::clear(uint32_t currentFrame) {
    m_buffers[currentFrame].clear();
}

RenderGraph::ImageUsage::ImageUsage(Node& node, vk::ImageLayout imageLayout, vk::AccessFlags accessMask, vk::PipelineStageFlags stageFlags) {
    m_node = &node;
    m_node->addUsage(*this);
    m_imageLayout = imageLayout;
    m_accessMask = accessMask;
    m_stageFlags = stageFlags;
    m_images.resize(node.graph().framesInFlight());
}

void RenderGraph::ImageUsage::sync(std::shared_ptr<Image> image, vk::ImageSubresourceRange subresource) {
    vk::Image* vkImage = &image->image();
    m_images[m_node->currentFrame()].insert({ vkImage, { image, vkImage, subresource } });
}

void RenderGraph::ImageUsage::clear(uint32_t currentFrame) {
    m_images[currentFrame].clear();
}

RenderGraph::Edge::Edge(Node& source, Node& dest) {
    m_sourceNode = &source;
    m_destNode = &dest;
}

RenderGraph::BufferEdge::BufferEdge(BufferUsage& sourceUsage, BufferUsage& destUsage) : Edge(sourceUsage.node(), destUsage.node()) {
    m_sourceUsage = &sourceUsage;
    m_destUsage = &destUsage;
}

void RenderGraph::BufferEdge::recordSourceBarriers(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    m_barriers.clear();
    vk::PipelineStageFlags sourceStageFlags = m_sourceUsage->stageFlags();
    vk::PipelineStageFlags destStageFlags = m_destUsage->stageFlags();

    if (source().queue().familyIndex() != dest().queue().familyIndex()) {
        destStageFlags = vk::PipelineStageFlags::BottomOfPipe;    //override dest stage flags when transfering queue ownership. recordDestBarriers will handle the other side
    }

    for (auto& sourcePair : m_sourceUsage->getSyncs(currentFrame)) {
        auto& destSyncs = m_destUsage->getSyncs(currentFrame);
        auto it = destSyncs.find(sourcePair.first);

        if (it != destSyncs.end()) {
            auto& sourceSegment = sourcePair.second;
            auto& destSegment = it->second;

            vk::BufferMemoryBarrier barrier = {};
            barrier.buffer = sourcePair.first;
            barrier.offset = sourceSegment.offset;
            barrier.size = sourceSegment.size;
            barrier.srcAccessMask = m_sourceUsage->accessMask();

            if (source().queue().familyIndex() == dest().queue().familyIndex()) {
                barrier.dstAccessMask = m_destUsage->accessMask();
            }

            barrier.srcQueueFamilyIndex = source().queue().familyIndex();
            barrier.dstQueueFamilyIndex = dest().queue().familyIndex();

            m_barriers.push_back(barrier);
        }
    }

    if (m_barriers.size() > 0) {
        commandBuffer.pipelineBarrier(m_sourceUsage->stageFlags(), m_destUsage->stageFlags(), {},
            nullptr,
            m_barriers,
            nullptr
        );
    }
}

void RenderGraph::BufferEdge::recordDestBarriers(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    if (source().queue().familyIndex() == dest().queue().familyIndex()) return;
    m_barriers.clear();
    vk::PipelineStageFlags sourceStageFlags = m_sourceUsage->stageFlags();
    vk::PipelineStageFlags destStageFlags = m_destUsage->stageFlags();

    if (source().queue().familyIndex() != dest().queue().familyIndex()) {
        sourceStageFlags = vk::PipelineStageFlags::TopOfPipe;    //override source stage flags when transfering queue ownership. recordDestBarriers will handle the other side
    }

    for (auto& sourcePair : m_sourceUsage->getSyncs(currentFrame)) {
        auto& destSyncs = m_destUsage->getSyncs(currentFrame);
        auto it = destSyncs.find(sourcePair.first);

        if (it != destSyncs.end()) {
            auto& sourceSegment = sourcePair.second;
            auto& destSegment = it->second;

            vk::BufferMemoryBarrier barrier = {};
            barrier.buffer = sourcePair.first;
            barrier.offset = sourceSegment.offset;
            barrier.size = sourceSegment.size;

            if (source().queue().familyIndex() == dest().queue().familyIndex()) {
                barrier.srcAccessMask = m_sourceUsage->accessMask();
            }

            barrier.dstAccessMask = m_destUsage->accessMask();
            barrier.srcQueueFamilyIndex = source().queue().familyIndex();
            barrier.dstQueueFamilyIndex = dest().queue().familyIndex();

            m_barriers.push_back(barrier);
        }
    }

    if (m_barriers.size() > 0) {
        commandBuffer.pipelineBarrier(m_sourceUsage->stageFlags(), m_destUsage->stageFlags(), {},
            nullptr,
            m_barriers,
            nullptr
        );
    }
}

RenderGraph::ImageEdge::ImageEdge(ImageUsage& sourceUsage, ImageUsage& destUsage) : Edge(sourceUsage.node(), destUsage.node()) {
    m_sourceUsage = &sourceUsage;
    m_destUsage = &destUsage;
}

void RenderGraph::ImageEdge::recordSourceBarriers(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    m_barriers.clear();
    vk::PipelineStageFlags sourceStageFlags = m_sourceUsage->stageFlags();
    vk::PipelineStageFlags destStageFlags = m_destUsage->stageFlags();

    if (source().queue().familyIndex() != dest().queue().familyIndex()) {
        destStageFlags = vk::PipelineStageFlags::BottomOfPipe;    //override dest stage flags when transfering queue ownership. recordDestBarriers will handle the other side
    }

    for (auto& sourcePair : m_sourceUsage->getSyncs(currentFrame)) {
        auto& destSyncs = m_destUsage->getSyncs(currentFrame);
        auto it = destSyncs.find(sourcePair.first);

        if (it != destSyncs.end()) {
            auto& sourceSegment = sourcePair.second;
            auto& destSegment = it->second;

            vk::ImageMemoryBarrier barrier = {};
            barrier.image = sourcePair.first;
            barrier.oldLayout = m_sourceUsage->imageLayout();
            barrier.newLayout = m_destUsage->imageLayout();
            barrier.subresourceRange = sourceSegment.subresource;
            barrier.srcAccessMask = m_sourceUsage->accessMask();

            if (source().queue().familyIndex() == dest().queue().familyIndex()) {
                barrier.dstAccessMask = m_destUsage->accessMask();
            }

            barrier.srcQueueFamilyIndex = source().queue().familyIndex();
            barrier.dstQueueFamilyIndex = dest().queue().familyIndex();

            m_barriers.push_back(barrier);
        }
    }

    if (m_barriers.size() > 0) {
        commandBuffer.pipelineBarrier(m_sourceUsage->stageFlags(), m_destUsage->stageFlags(), {},
            nullptr,
            nullptr,
            m_barriers
        );
    }
}

void RenderGraph::ImageEdge::recordDestBarriers(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    if (source().queue().familyIndex() == dest().queue().familyIndex()) return;
    m_barriers.clear();
    vk::PipelineStageFlags sourceStageFlags = m_sourceUsage->stageFlags();
    vk::PipelineStageFlags destStageFlags = m_destUsage->stageFlags();

    if (source().queue().familyIndex() != dest().queue().familyIndex()) {
        sourceStageFlags = vk::PipelineStageFlags::TopOfPipe;    //override source stage flags when transfering queue ownership. recordDestBarriers will handle the other side
    }

    for (auto& sourcePair : m_sourceUsage->getSyncs(currentFrame)) {
        auto& destSyncs = m_destUsage->getSyncs(currentFrame);
        auto it = destSyncs.find(sourcePair.first);

        if (it != destSyncs.end()) {
            auto& sourceSegment = sourcePair.second;
            auto& destSegment = it->second;

            vk::ImageMemoryBarrier barrier = {};
            barrier.image = sourcePair.first;
            barrier.oldLayout = m_sourceUsage->imageLayout();
            barrier.newLayout = m_destUsage->imageLayout();
            barrier.subresourceRange = sourceSegment.subresource;

            if (source().queue().familyIndex() == dest().queue().familyIndex()) {
                barrier.srcAccessMask = m_sourceUsage->accessMask();
            }

            barrier.dstAccessMask = m_destUsage->accessMask();
            barrier.srcQueueFamilyIndex = source().queue().familyIndex();
            barrier.dstQueueFamilyIndex = dest().queue().familyIndex();

            m_barriers.push_back(barrier);
        }
    }

    if (m_barriers.size() > 0) {
        commandBuffer.pipelineBarrier(m_sourceUsage->stageFlags(), m_destUsage->stageFlags(), {},
            nullptr,
            nullptr,
            m_barriers
        );
    }
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

void RenderGraph::Node::addUsage(BufferUsage& usage) {
    m_bufferUsages.push_back(&usage);
}

void RenderGraph::Node::addUsage(ImageUsage& usage) {
    m_imageUsages.push_back(&usage);
}

void RenderGraph::Node::addOutput(Node& output, Edge& edge) {
    m_outputNodes.push_back(&output);
    m_outputEdges.push_back(&edge);
    output.m_inputEdges.push_back(&edge);
}

void RenderGraph::Node::makeInputTransfers(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    for (auto& edge : m_inputEdges) {
        edge->recordDestBarriers(currentFrame, commandBuffer);
    }
}

void RenderGraph::Node::makeOutputTransfers(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) {
    for (auto& edge : m_outputEdges) {
        edge->recordSourceBarriers(currentFrame, commandBuffer);
    }
}

void RenderGraph::Node::wait(uint32_t currentFrame) {
    vk::Fence& fence = m_fences[currentFrame];
    fence.wait();
    fence.reset();

    m_currentFrame = currentFrame;
}

void RenderGraph::Node::clearSync(uint32_t currentFrame) {
    for (auto usage : m_bufferUsages) {
        usage->clear(currentFrame);
    }

    for (auto usage : m_imageUsages) {
        usage->clear(currentFrame);
    }
}

void RenderGraph::Node::internalRender(uint32_t currentFrame) {
    vk::CommandBuffer& commandBuffer = m_commandBuffers[currentFrame];

    commandBuffer.reset(vk::CommandBufferResetFlags::None);

    vk::CommandBufferBeginInfo info = {};
    info.flags = vk::CommandBufferUsageFlags::OneTimeSubmit;

    commandBuffer.begin(info);

    makeInputTransfers(currentFrame, commandBuffer);
    render(currentFrame, commandBuffer);
    makeOutputTransfers(currentFrame, commandBuffer);

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

void RenderGraph::addEdge(BufferEdge&& edge) {
    m_edges.emplace_back(std::make_unique<BufferEdge>(std::move(edge)));
    auto& edge_ = m_edges.back();
    edge_->source().addOutput(edge_->dest(), *edge_);
}

void RenderGraph::addEdge(ImageEdge&& edge) {
    m_edges.emplace_back(std::make_unique<ImageEdge>(std::move(edge)));
    auto& edge_ = m_edges.back();
    edge_->source().addOutput(edge_->dest(), *edge_);
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
        node->wait(m_currentFrame);
    }
    for (auto node : m_nodeList) {
        node->clearSync(m_currentFrame);
    }

    for (auto node : m_nodeList) {
        node->preRender(m_currentFrame);
    }

    size_t i = 0;
    for (auto node : m_nodeList) {
        node->internalRender(m_currentFrame);
        i++;
    }

    for (auto node : m_nodeList) {
        node->submit(m_currentFrame);
    }

    for (auto node : m_nodeList) {
        node->postRender(m_currentFrame);
    }

    m_currentFrame = (m_currentFrame + 1) % m_framesInFlight;
}