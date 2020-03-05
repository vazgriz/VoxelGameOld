#include "BlockManager.h"

BlockType::BlockType(uint32_t id, const FaceArray& faces, bool solid) {
    m_id = id;
    m_faces = faces;
    m_solid = solid;
}

bool BlockType::solid() const {
    return m_solid;
}

BlockManager::BlockManager() {
    m_types.emplace_back(0, BlockType::FaceArray{}, false);
    m_types.emplace_back(1, BlockType::FaceArray{}, false);
    m_types.emplace_back(2, BlockType::FaceArray{ 0, 0, 0, 0, 0, 0 });
    m_types.emplace_back(3, BlockType::FaceArray{ 1, 1, 2, 0, 1, 1 });
    m_types.emplace_back(4, BlockType::FaceArray{ 3, 3, 3, 3, 3, 3 });
}