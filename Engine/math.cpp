#include "Engine/math.h"

int32_t VoxelEngine::distance2(glm::ivec3 a, glm::ivec3 b) {
    glm::ivec3 diff = a - b;
    return (diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z);
}