#include "wolf/render/FrameBuffer.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace wolf {

FrameBuffer::FrameBuffer(int width, int height)
    : width_(width), height_(height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("FrameBuffer size must be positive");
    }

    pixels_.resize(static_cast<std::size_t>(width) * height);
}

void FrameBuffer::clear(Color color) {
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void FrameBuffer::setPixel(int x, int y, Color color) {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return;
    }

    pixels_[static_cast<std::size_t>(y) * width_ + x] = color;
}

void FrameBuffer::fillRect(int x, int y, int w, int h, Color color) {
    if (w <= 0 || h <= 0) return;

    const int left = std::max(x, 0);
    const int right = static_cast<int>(
        std::min(1LL * x + w, 1LL * width_)
    );

    const int top = std::max(y, 0);
    const int bottom = static_cast<int>(
        std::min(1LL * y + h, 1LL * height_)
    );

    if (left >= right || top >= bottom) return;

    for (int py = top; py < bottom; ++py) {
        const auto row = static_cast<std::size_t>(py) * width_;

        std::fill(
            pixels_.begin() + row + left,
            pixels_.begin() + row + right,
            color
        );
    }
}

void FrameBuffer::drawColumn(
    int x, int yBegin, int yEnd, Color color
) {
    if (x < 0 || x >= width_) return;

    const int top = std::max(yBegin, 0);
    const int bottom = std::min(yEnd, height_);

    for (int y = top; y < bottom; ++y) {
        pixels_[static_cast<std::size_t>(y) * width_ + x] = color;
    }
}

}
