#include "Chunk.h"

Chunk::PositionIterator::PositionIterator() {
    state = {};
}

Chunk::PositionIterator::PositionIterator(const Chunk::PositionIterator& other) {
    state = other.state;
}

Chunk::PositionIterator& Chunk::PositionIterator::operator=(const Chunk::PositionIterator& other) {
    state = other.state;
    return *this;
}

bool Chunk::PositionIterator::operator==(const Chunk::PositionIterator& other) const {
    return state == other.state;
}

bool Chunk::PositionIterator::operator!=(const Chunk::PositionIterator& other) const {
    return state != other.state;
}

Chunk::PositionIterator& Chunk::PositionIterator::operator++() {
    int32_t carry = increment(state.y, increment(state.x, 1));
    state.z += carry;
    return *this;
}

Chunk::PositionIterator::reference Chunk::PositionIterator::operator*() const {
    return state;
}

Chunk::PositionIterator::pointer Chunk::PositionIterator::operator->() const {
    return &state;
}

int32_t Chunk::PositionIterator::increment(int32_t& dest, int32_t value) {
    int32_t temp = dest + value;
    dest = temp & mask;
    return temp >> shiftAmount;
}

Chunk::PositionIterator::PositionIterator(glm::ivec3 state) {
    this->state = state;
}

Chunk::Chunk(glm::ivec3 pos) {
    m_chunkPosition = pos;
}

size_t Chunk::index(glm::ivec3 pos) {
    return pos.x + (pos.y * chunkSize) + (pos.z * chunkSize * chunkSize);
}

glm::ivec3 Chunk::position(size_t index) {
    glm::ivec3 result = {};
    result.x = static_cast<int32_t>(index & mask);
    index = index >> shiftAmount;
    result.y = static_cast<int32_t>(index & mask);
    index = index >> shiftAmount;
    result.z = static_cast<int32_t>(index & mask);

    return result;
}