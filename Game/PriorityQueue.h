#pragma once
#include <vector>
#include <unordered_set>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <Engine/math.h>
#include <entt/entt.hpp>
#include <algorithm>
#include <limits>

template <typename T>
class PriorityQueue {
public:
    struct Item {
        glm::ivec3 pos;
        T data;
        int32_t priority;

        bool operator < (const Item& other) {
            return priority > other.priority;
        }
    };

    PriorityQueue() {
        int32_t max = std::numeric_limits<int32_t>::max();
        m_playerPos = { max, max, max };
    }

    size_t count() const { return m_set.size(); }

    void enqueue(glm::ivec3 pos, T data) {
        if (m_set.insert(pos).second) {
            m_items.push_back({ pos, data, VoxelEngine::distance2(pos, m_playerPos) });
            std::push_heap(m_items.begin(), m_items.end());
        }
    }

    Item peek() {
        while (true) {
            Item item = m_items.front();

            auto it = m_set.find(item.pos);
            if (it == m_set.end()) {
                std::pop_heap(m_items.begin(), m_items.end());
                m_items.pop_back();
            } else {
                return item;
            }
        }
    }

    Item dequeue() {
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

    void remove(glm::ivec3 pos) {
        m_set.erase(pos);
    }

    void update(glm::ivec3 pos) {
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

private:
    std::vector<Item> m_items;
    std::unordered_set<glm::ivec3> m_set;
    glm::ivec3 m_playerPos;
};