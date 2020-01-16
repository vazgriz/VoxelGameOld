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

        class BufferInput {
        public:
            BufferInput(Node& node, vk::AccessFlags accessMask);

            Node& node() const { return *m_node; }
            vk::AccessFlags flags() const { return m_dstFlags; }
            vk::PipelineStageFlags stages() const { return m_dstStages; }

        private:
            Node* m_node;
            vk::AccessFlags m_dstFlags;
            vk::PipelineStageFlags m_dstStages;
        };

        class BufferOutput {
        public:
            BufferOutput(Node& node, vk::AccessFlags accessMask);

            Node& node() const { return *m_node; }
            vk::AccessFlags flags() const { return m_srcFlags; }
            vk::PipelineStageFlags stages() const { return m_srcStages; }

        private:
            Node* m_node;
            vk::AccessFlags m_srcFlags;
            vk::PipelineStageFlags m_srcStages;
        };

        class ImageInput {
        public:
            ImageInput(Node& node, vk::AccessFlags accessMask);

            Node& node() const { return *m_node; }
            vk::AccessFlags flags() const { return m_dstFlags; }
            vk::PipelineStageFlags stages() const { return m_dstStages; }

        private:
            Node* m_node;
            vk::AccessFlags m_dstFlags;
            vk::PipelineStageFlags m_dstStages;
        };

        class ImageOutput {
        public:
            ImageOutput(Node& node, vk::AccessFlags accessMask);

            Node& node() const { return *m_node; }
            vk::AccessFlags flags() const { return m_srcFlags; }
            vk::PipelineStageFlags stages() const { return m_srcStages; }

        private:
            Node* m_node;
            vk::AccessFlags m_srcFlags;
            vk::PipelineStageFlags m_srcStages;
        };

        class Edge {
        public:
            Edge(Node& source, Node& dest);

            Node& source() const { return *m_sourceNode; }
            Node& dest() const { return *m_destNode; }

        private:
            Node* m_sourceNode;
            Node* m_destNode;
        };

        class BufferEdge : public Edge {
        public:
            BufferEdge(const BufferOutput& source, const BufferInput& dest);

        private:
            const BufferOutput* m_source;
            const BufferInput* m_dest;
        };

        class ImageEdge : public Edge {
        public:
            ImageEdge(const ImageOutput& source, const ImageInput& dest);

        private:
            const ImageOutput* m_source;
            const ImageInput* m_dest;
        };

        class Node {
            friend class RenderGraph;

        public:
            Node(RenderGraph& graph, const vk::Queue& queue, vk::PipelineStageFlags stages);
            virtual ~Node() = default;

            RenderGraph& graph() const { return *m_graph; }

            void addExternalWait(vk::Semaphore& semaphore, vk::PipelineStageFlags stages);
            void addExternalSignal(vk::Semaphore& semaphore);

            virtual void preRender(uint32_t currentFrame) = 0;
            virtual void render(uint32_t currentFrame, vk::CommandBuffer& commandBuffer) = 0;
            virtual void postRender(uint32_t currentFrame) = 0;

        protected:
            void addBufferInput(const BufferInput& input);
            void addBufferOutput(const BufferOutput& output);
            void addImageInput(const ImageInput& input);
            void addImageOutput(const ImageOutput& output);
            vk::CommandPool& commandPool() const { return *m_commandPool; }

        private:
            const vk::Queue* m_queue;
            vk::PipelineStageFlags m_stages;
            RenderGraph* m_graph;
            std::vector<Node*> m_outputNodes;
            std::unordered_set<Node*> m_outputSet;
            std::vector<const BufferInput*> m_bufferInputs;
            std::vector<const BufferOutput*> m_bufferOutputs;
            std::vector<const ImageInput*> m_imageInputs;
            std::vector<const ImageOutput*> m_imageOutputs;

            std::vector<vk::Fence> m_fences;
            std::unique_ptr<vk::CommandPool> m_commandPool;
            std::vector<vk::CommandBuffer> m_commandBuffers;
            vk::SubmitInfo m_submitInfo;

            void addOutput(Node& output);

            void internalRender(uint32_t currentFrame);
            void submit(uint32_t currentFrame);
            void wait();
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
        std::vector<std::unique_ptr<Edge>> m_edges;
        std::vector<Node*> m_nodeList;
        std::vector<vk::Semaphore> m_semaphores;

        void makeSemaphores();
    };
}