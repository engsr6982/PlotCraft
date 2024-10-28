#pragma once
#include <cstddef>
#include <cstdio>
#include <functional>
#include <optional>
#include <queue>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>


class NodeTree {
public:
    using NodePair = std::pair<int, int>;

    struct Node {
        int    x, z;          // 坐标
        double distance{0.0}; // 距离

        Node() = delete;
        Node(int x, int z) : x(x), z(z) {}
        Node(NodePair const& p) : x(p.first), z(p.second) {}
        Node(int x, int z, double distance) : x(x), z(z), distance(distance) {}

        template <typename T = Node>
        Node(T const& cl) {
            static_assert(std::is_class<T>::value, "T must be a class");

            // 检查T是否有int成员x,z
            static_assert(std::is_member_object_pointer<decltype(&T::x)>::value, "T must have an int member x");
            static_assert(std::is_member_object_pointer<decltype(&T::z)>::value, "T must have an int member z");
            static_assert(std::is_same<decltype(T::x), int>::value, "x must be of type int");
            static_assert(std::is_same<decltype(T::z), int>::value, "z must be of type int");

            this->x = cl.x;
            this->z = cl.z;
        }

        operator NodePair() const { return {x, z}; }
        operator std::string() const { return "Node(" + std::to_string(x) + ", " + std::to_string(z) + ")"; }
        bool operator<(Node const& other) const {
            return distance > other.distance; // 优先按距离排序
        }
    };

    struct PairHash {
        std::size_t operator()(NodePair const& p) const {
            return std::hash<int>()(p.first) ^ std::hash<int>()(p.second);
        }
    };


    // member
    std::unordered_set<NodePair, PairHash> nodes; // 节点集合

    // method
    bool contains(NodePair const& p) const { return nodes.contains(p); }
    bool contains(int x, int z) const { return contains({x, z}); }

    void insert(NodePair const& p) { nodes.insert(p); }
    void insert(int x, int z) { insert({x, z}); }

    void remove(NodePair const& p) { nodes.erase(p); }
    void remove(int x, int z) { remove({x, z}); }

    void clear() { nodes.clear(); }

    // 查找距离 root 最近的未标记节点
    std::optional<Node> findNearestUnmarkedNodeFromRoot(NodePair const& root = {0, 0}) {
        static std::vector<NodePair> directions = {
            {1,  0 },
            {-1, 0 },
            {0,  1 },
            {0,  -1}
        };

        // 优先队列初始化
        std::priority_queue<Node> pq;
        pq.push(root);

        // 记录已访问的节点
        std::unordered_set<NodePair, PairHash> visited;
        visited.emplace(root);

        while (!pq.empty()) {
            Node current = pq.top();
            pq.pop();

            // 检查是否未被标记
            if (nodes.find({current.x, current.z}) == nodes.end()) {
                return current;
            }

            // 将邻近的节点加入优先队列
            for (const auto& dir : directions) {
                int newX = current.x + dir.first;
                int newZ = current.z + dir.second;
                if (visited.find({newX, newZ}) == visited.end()) {
                    double dist = std::sqrt(newX * newX + newZ * newZ);
                    pq.push(Node{newX, newZ, dist});
                    visited.emplace(newX, newZ);
                }
            }
        }
        return std::nullopt;
    }
};


#include <iostream>

// cl /EHsc /utf-8 /std:c++20 .\test.cc

int main() {
    NodeTree tree;
    tree.insert(0, 0);
    tree.insert(1, 0);
    tree.insert(0, 1);
    tree.insert(0, -1);
    tree.insert(-1, 0);
    // tree.insert(1, 1);
    tree.insert(1, -1);
    tree.insert(-1, 1);
    tree.insert(-1, -1);

    auto nearest = tree.findNearestUnmarkedNodeFromRoot();
    if (nearest) {
        std::cout << "Nearest unmarked node: " << (std::string)(*nearest) << std::endl;
    } else {
        std::cout << "No unmarked node found." << std::endl;
    }

    {
        class a {
        public:
            int x = 11;
            int z = 45;
        };

        struct b {
            int x, z = 8848;
        };

        NodeTree::Node a1{a{}};
        NodeTree::Node b1{b{}};

        std::cout << (std::string)a1 << std::endl;
        std::cout << (std::string)b1 << std::endl;
    }

    std::cout << std::endl;
    return 0;
}