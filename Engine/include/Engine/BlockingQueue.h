#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>

namespace VoxelEngine {
    template <typename T>
    class BlockingQueue {
    public:
        BlockingQueue(size_t count) {
            m_maxCount = count;
        }

        bool tryEnqueue(T item) {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_queue.size() == m_maxCount) return false;

            bool empty = m_queue.empty();
            m_queue.push(item);

            lock.unlock();

            if (empty) {
                m_condition.notify_one();
            }

            return true;
        }

        bool dequeue(T& result) {
            std::unique_lock<std::mutex> lock(m_mutex);

            while (true) {
                if (m_cancel) return false;

                if (m_queue.empty()) {
                    m_condition.wait(m_mutex);
                }
            }

            result = m_queue.front();
            m_queue.pop();

            return true;
        }

        void cancel() {
            std::unique_lock<std::mutex> lock(m_mutex);

            m_cancel = true;
            lock.unlock();
            m_condition.notify_all();
        }

    private:
        std::queue<T> m_queue;
        std::mutex m_mutex;
        std::condition_variable m_condition;
        size_t m_maxCount;
        bool m_cancel;
    };
}