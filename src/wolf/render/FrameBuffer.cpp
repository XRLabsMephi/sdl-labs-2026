#include "wolf/render/FrameBuffer.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

FrameBuffer::FrameBuffer(int width, int height)
    : width_(width), height_(height) {
    if (width_ <= 0 || height_ <= 0) {
        throw std::invalid_argument("FrameBuffer dimensions must be positive");
    }

    pixels_.resize(
        static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_)
    );
}

int FrameBuffer::width() const {
    return width_;
}

int FrameBuffer::height() const {
    return height_;
}

void FrameBuffer::clear(Color color) {
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void FrameBuffer::setPixel(int x, int y, Color color) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return;
    }
    pixels_[static_cast<std::size_t>(y) * width_ + x] = color;
}

const Color* FrameBuffer::data() const {
    return pixels_.data();
}