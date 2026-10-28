#include "wolf/render/FrameBuffer.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace wolf {

    namespace {

        std::size_t checkedPixelCount(int width, int height) {
            if (width <= 0 || height <= 0) {
                throw std::invalid_argument("FrameBuffer dimensions must be positive");
            }
            const auto w = static_cast<std::size_t>(width);
            const auto h = static_cast<std::size_t>(height);
            if (w > std::vector<Color>().max_size() / h) {
                throw std::length_error("FrameBuffer is too large");
            }
            return w * h;
        }

    }

    FrameBuffer::FrameBuffer(int width, int height)
        : width_(width),
        height_(height),
        pixels_(checkedPixelCount(width, height)) {}

    int FrameBuffer::width() const noexcept { return width_; }
    int FrameBuffer::height() const noexcept { return height_; }
    const Color* FrameBuffer::pixels() const noexcept { return pixels_.data(); }

    void FrameBuffer::clear(Color color) noexcept {
        std::fill(pixels_.begin(), pixels_.end(), color);
    }

    void FrameBuffer::fillRows(int firstRow, int endRow, Color color) noexcept {
        firstRow = std::clamp(firstRow, 0, height_);
        endRow = std::clamp(endRow, 0, height_);
        if (firstRow >= endRow) { return; }
        const auto first = static_cast<std::size_t>(firstRow) * static_cast<std::size_t>(width_);
        const auto last = static_cast<std::size_t>(endRow) * static_cast<std::size_t>(width_);
        std::fill(pixels_.begin() + static_cast<std::ptrdiff_t>(first),
                  pixels_.begin() + static_cast<std::ptrdiff_t>(last),
                  color);
    }

    bool FrameBuffer::inBounds(int x, int y) const noexcept {
        return x >= 0 && x < width_ && y >= 0 && y < height_;
    }

    void FrameBuffer::setPixel(int x, int y, Color color) noexcept {
        if (!inBounds(x, y)) { return; }
        const auto index = static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) +
                           static_cast<std::size_t>(x);
        pixels_[index] = color;
    }

    Color FrameBuffer::pixelAt(int x, int y) const {
        if (!inBounds(x, y)) {
            throw std::out_of_range("FrameBuffer pixel out of bounds");
        }
        const auto index = static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) +
                           static_cast<std::size_t>(x);
        return pixels_[index];
    }

    void FrameBuffer::drawVerticalSpan(int x, int firstRow, int endRow, Color color) noexcept {
        if (x < 0 || x >= width_) { return; }
        firstRow = std::clamp(firstRow, 0, height_);
        endRow = std::clamp(endRow, 0, height_);
        if (firstRow >= endRow) { return; }
        const auto column = static_cast<std::size_t>(x);
        const auto stride = static_cast<std::size_t>(width_);
        for (int y = firstRow; y < endRow; ++y) {
            pixels_[static_cast<std::size_t>(y) * stride + column] = color;
        }
    }

}
