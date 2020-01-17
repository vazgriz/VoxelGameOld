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
        };

        class BufferInput {
            friend class Node;
        public:
            BufferInput(Node& node, vk::AccessFlags accessMask, vk::PipelineStageFlags stages);

            Node& node() const { return *m_node; }
            vk::AccessFlags flags() const { return m_dstFlags; }
            vk::PipelineStageFlags stages() const { return m_dstStages; }

            void input(const Buffer& buffer);

        private:
            Node* m_node;
            vk::AccessFlags m_dstFlags;
            vk::PipelineStageFlags m_dstStages;
            std::unordered_map<const Buffer*, BufferSegment> m_inputs;

            std::unordered_map<const Buffer*, BufferSegment>& inputs() { return m_inputs; }
            void clear();
        };

        class BufferOutput {
            friend class Node;
        public:
            BufferOutput(Node& node, vk::AccessFlags accessMask, vk::PipelineStageFlags stages);

            Node& node() const { return *m_node; }
            vk::AccessFlags flags() const { return m_srcFlags; }
            vk::PipelineStageFlags stages() const { return m_srcStages; }

            void output(const Buffer& buffer);

        private:
            Node* m_node;
            vk::AccessFlags m_srcFlags;
            vk::PipelineStageFlags m_srcStages;
            std::unordered_map<const Buffer*, BufferSegment> m_outputs;

            std::unordered_map<const Buffer*, BufferSegment>& outputs() { return m_outputs; }
            void clear();
        };

        class ImageInput {
            friend class Node;
        public:
            ImageInput(Node& node, vk::AccessFlags accessMask, vk::PipelineStageFlags stages);

            Node& node() const { return *m_node; }
            vk::AccessFlags flags() const { return m_dstFlags; }
            vk::PipelineStageFlags stages() const { return m_dstStages; }

        private:
            Node* m_node;
            vk::AccessFlags m_dstFlags;
            vk::PipelineStageFlags m_dstStages;

            void clear();
        };

        class ImageOutput {
            friend class Node;
        public:
            ImageOutput(Node& node, vk::AccessFlags accessMask, vk::PipelineStageFlags stages);

            Node& node() const { return *m_node; }
            vk::AccessFlags flags() const { return m_srcFlags; }
            vk::PipelineStageFlags stages() const { return m_srcStages; }

        private:
            Node* m_node;
            vk::AccessFlags m_srcFlags;
            vk::PipelineStageFlags m_srcStages;

            void clear();
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

            virtual void makeTransferEdge() {};
        };

        class BufferEdge : public Edge {
        public:
            BufferEdge(BufferOutput& source, BufferInput& dest);

            BufferOutput& sourceOutput() const { return *m_source; }
            BufferInput& destInput() const { return *m_dest; }

        private:
            BufferOutput* m_source;
            BufferInput* m_dest;

            void makeTransferEdge();
        };

        class ImageEdge : public Edge {
        public:
            ImageEdge(ImageOutput& source, ImageInput& dest);

            ImageOutput& sourceOutput() const { return *m_source; }
            ImageInput& destInput() const { return *m_dest; }

        private:
            ImageOutput* m_source;
            ImageInput* m_dest;

            void makeTransferEdge();
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
            void addBufferInput(BufferInput& input);
            void addBufferOutput(BufferOutput& output);
            void addImageInput(ImageInput& input);
            void addImageOutput(ImageOutput& output);
            vk::CommandPool& commandPool() const { return *m_commandPool; }

        private:
            const vk::Queue* m_queue;
            vk::PipelineStageFlags m_stages;
            RenderGraph* m_graph;
            std::vector<BufferEdge*> m_outputBufferEdges;
            std::vector<BufferEdge*> m_inputBufferEdges;
            std::vector<ImageEdge*> m_outputImageEdges;
            std::vector<ImageEdge*> m_inputImageEdges;
            std::vector<Node*> m_outputNodes;
            std::unordered_set<Node*> m_outputSet;
            std::vector<BufferInput*> m_bufferInputs;
            std::vector<BufferOutput*> m_bufferOutputs;
            std::vector<ImageInput*> m_imageInputs;
            std::vector<ImageOutput*> m_imageOutputs;

            std::vector<vk::Fence> m_fences;
            std::unique_ptr<vk::CommandPool> m_commandPool;
            std::vector<vk::CommandBuffer> m_commandBuffers;
            vk::SubmitInfo m_submitInfo;

            void addInputEdge(BufferEdge& edge);
            void addOutputEdge(BufferEdge& edge);
            void addInputEdge(ImageEdge& edge);
            void addOutputEdge(ImageEdge& edge);
            void addOutput(Node& output);

            void makeInputTransfers(vk::CommandBuffer& commandBuffer);
            void makeOutputTransfers(vk::CommandBuffer& commandBuffer);
            void internalRender(uint32_t currentFrame);
            void submit(uint32_t currentFrame);
            void wait();
            void clearInputsAndOutputs();
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

        void addEdge(std::unique_ptr<Edge> edge);
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
        std::vector<std::unique_ptr<vk::Semaphore>> m_semaphores;
        std::vector<Edge*> m_transferEdges;

        void makeSemaphores();
    };
}