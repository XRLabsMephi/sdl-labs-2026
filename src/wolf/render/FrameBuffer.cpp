#include "wolf/render/FrameBuffer.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace wolf {

FrameBuffer::FrameBuffer(int width, int height)
    : width_(width), height_(height) {
    if (width <= 0 || height <= 0)
        throw std::invalid_argument("FrameBuffer size must be positive");

    pixels_.resize(static_cast<std::size_t>(width) * height);
}

void FrameBuffer::clear(Color color) {
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void FrameBuffer::setPixel(int x, int y, Color color) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) return;

    pixels_[y * width_ + x] = color;
}

}
