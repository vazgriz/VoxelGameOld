#pragma once
#include "Engine/Engine.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

namespace VoxelEngine {
    class RenderGraph {
    public:
        class Node;
        class Edge;

        struct BufferSegment {
            vk::Buffer* buffer;
            vk::DeviceSize size;
            vk::DeviceSize offset;
            vk::AccessFlags accessMask;
            vk::PipelineStageFlags stages;
        };

        class Edge {
            friend class RenderGraph;
        public:
            Edge(Node& source, Node& dest);

            Node& source() const { return *m_sourceNode; }
            Node& dest() const { return *m_destNode; }

        private:
            Node* m_sourceNode;
            Node* m_destNode;
        };

        class Node {
            friend class RenderGraph;

        public:
            Node(RenderGraph& graph, const vk::Queue& queue, vk::PipelineStageFlags stages);
            virtual ~Node() = default;

            RenderGraph& graph() const { return *m_graph; }
            const vk::Queue& queue() const { return *m_queue; }

            void addExternalWait(vk::Semaphore& semaphore, vk::PipelineStageFlags stages);
            void addExternalSignal(vk::Semaphore& semaphore);

            virtual void preRender(uint32_t currentFrame) = 0;
            virtual void render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) = 0;
            virtual void postRender(uint32_t currentFrame) = 0;

        protected:
            vk::CommandPool& commandPool() const { return *m_commandPool; }
            const std::unordered_map<vk::Buffer*, BufferSegment>& getSyncBuffers() { return m_syncBuffers; }

            void sync(const Buffer& buffer);

        private:
            const vk::Queue* m_queue;
            vk::PipelineStageFlags m_stages;
            RenderGraph* m_graph;
            std::vector<Node*> m_outputNodes;
            std::unordered_set<Node*> m_outputSet;
            std::vector<Edge*> m_inputEdges;
            std::vector<Edge*> m_outputEdges;

            std::vector<vk::Fence> m_fences;
            std::unique_ptr<vk::CommandPool> m_commandPool;
            std::vector<vk::CommandBuffer> m_commandBuffers;
            vk::SubmitInfo m_submitInfo;

            std::unordered_map<vk::Buffer*, BufferSegment> m_syncBuffers;

            void addOutput(Node& output);

            void makeInputTransfers(vk::CommandBuffer& commandBuffer);
            void makeOutputTransfers(vk::CommandBuffer& commandBuffer);
            void internalRender(uint32_t currentFrame);
            void submit(uint32_t currentFrame);
            void wait();
            void clearSync();
        };

        RenderGraph(vk::Device& device, uint32_t framesInFlight);

        vk::Device& device() const { return *m_device; }
        uint32_t framesInFlight() const { return m_framesInFlight; }
        uint32_t currentFrame() const { return m_currentFrame; }

        template<class T, class... Args>
        T& addNode(Args&&... args) {
            m_nodes.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
            return *static_cast<T*>(m_nodes.back().get());
        }

        void addEdge(Edge&& edge);
        void bake();
        void wait();

        void execute() const;

    private:
        vk::Device* m_device;
        uint32_t m_framesInFlight;
        mutable uint32_t m_currentFrame;
        std::vector<std::unique_ptr<Node>> m_nodes;
        std::vector<Edge> m_edges;
        std::vector<Node*> m_nodeList;
        std::vector<std::unique_ptr<vk::Semaphore>> m_semaphores;
        std::vector<Edge*> m_transferEdges;

        void makeSemaphores();
    };
}