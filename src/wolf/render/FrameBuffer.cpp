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

void FrameBuffer::fillRect(int x, int y, int w, int h, Color color) {
    if (w <= 0 || h <= 0) return;

    int left = std::max(x, 0);
    int right = std::min(x + w, width_);
    int top = std::max(y, 0);
    int bottom = std::min(y + h, height_);

    for (int py = top; py < bottom; ++py) {
        for (int px = left; px < right; ++px) {
            setPixel(px, py, color);
        }   
    }
}

void FrameBuffer::drawColumn(int x, int yBegin, int yEnd, Color color) {
    if (x < 0 || x >= width_) return;

    int top = std::max(yBegin, 0);
    int bottom = std::min(yEnd, height_);

    for (int y = top; y < bottom; ++y) setPixel(x, y, color);
}

}
