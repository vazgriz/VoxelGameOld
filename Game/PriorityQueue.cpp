#include "PriorityQueue.h"
#include <algorithm>

bool PriorityQueue::Item::operator < (const Item& other) {
    return priority > other.priority;
}

PriorityQueue::PriorityQueue() {
    m_playerPos = {};
}

void PriorityQueue::enqueue(glm::ivec3 pos, entt::entity entity) {
    if (m_set.insert(pos).second) {
        m_items.push_back({ pos, entity });
        std::push_heap(m_items.begin(), m_items.end());
    }
}

PriorityQueue::Item PriorityQueue::peek() {
    while (true) {
        Item item = m_items.front();

        auto it = m_set.find(item.pos);
        if (it == m_set.end()) {
            std::pop_heap(m_items.begin(), m_items.end());
            m_items.pop_back();
            m_set.erase(it);
        } else {
            return item;
        }
    }
}

PriorityQueue::Item PriorityQueue::dequeue() {
    while (true) {
        Item item = m_items.front();
        std::pop_heap(m_items.begin(), m_items.end());
        m_items.pop_back();

        auto it = m_set.find(item.pos);
        if (it != m_set.end()) {
            m_set.erase(it);
            return item;
        }
    }
}

void PriorityQueue::remove(glm::ivec3 pos) {
    m_set.erase(pos);
}

void PriorityQueue::update(glm::ivec3 pos) {
    if (pos != m_playerPos) {
        m_playerPos = pos;

        for (auto& item : m_items) {
            auto it = m_set.find(item.pos);
            if (it != m_set.end()) {
                item.priority = VoxelEngine::distance2(pos, item.pos);
            }
        }

        std::make_heap(m_items.begin(), m_items.end());
    }
}