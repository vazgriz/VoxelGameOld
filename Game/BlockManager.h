#pragma once
#include <stdint.h>
#include <vector>
#include <array>

class BlockType {
public:
    using FaceArray = std::array<size_t, 6>;

    BlockType(uint32_t id, const FaceArray& faces);

    size_t getFaceIndex(size_t index) const { return m_faces[index]; }

private:
    uint32_t m_id;
    FaceArray m_faces;
};

class BlockManager {
public:
    BlockManager();

    size_t typeCount() const { return m_types.size(); }
    BlockType& getType(size_t id) { return m_types[id]; }

private:
    std::vector<BlockType> m_types;
};