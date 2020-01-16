#pragma once
#include <vector>
#include <unordered_set>
#include <functional>
#include <stack>

namespace VoxelEngine {
    template <typename T>
    std::vector<T*> topologicalSort(std::unordered_set<T*> graph, std::function<std::vector<T*>(T*)> outNodeFunctor) {
        std::unordered_set<T*> temporary;
        std::unordered_set<T*> permanent;
        std::stack<T*> stack;

        std::function<void(T*)> visit;
        visit = [&](T* vertex) {
            if (permanent.count(vertex) == 1) return;
            if (temporary.count(vertex) == 1) throw std::runtime_error("Not a Directed Acyclic Graph");

            temporary.insert(vertex);
            std::vector<T*> outNodes = outNodeFunctor(vertex);

            for (auto v : outNodes) {
                visit(v);
            }

            permanent.insert(vertex);
            stack.push(vertex);
        };

        for (auto node : graph) {
            visit(node);
        }

        std::vector<T*> list;

        while (stack.size() > 0) {
            list.push_back(stack.top());
            stack.pop();
        }

        return list;
    }
}