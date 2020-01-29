#pragma once
#include <mutex>
#include <queue>
#include <vector>

namespace VoxelEngine {
    template <typename T>
    class BufferedQueue {
    public:
        BufferedQueue() : m_queues(2) {
            m_index = 0;
        }

        void enqueue(T item) {
            std::unique_lock<std::mutex> lock(m_mutex);

            std::queue<T>& frontQueue = m_queues[m_index];
            frontQueue.push(item);
        }

        std::queue<T>& swapDequeue() {
            std::unique_lock<std::mutex> lock(m_mutex);
            std::queue<T>& backQueue = m_queues[m_index];
            m_index = (m_index + 1) & 1;

            return backQueue;
        }

    private:
        std::vector<std::queue<T>> m_queues;
        std::mutex m_mutex;
        size_t m_index;
    };
}