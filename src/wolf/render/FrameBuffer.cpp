#include "wolf/render/FrameBuffer.hpp"

#include <algorithm>
#include <limits>
#include <stdexcept>

namespace wolf {

FrameBuffer::FrameBuffer(int width, int height) : width_(width), height_(height) {
    if (width <= 0 || height <= 0
        || width > std::numeric_limits<int>::max() / int(sizeof(std::uint32_t))) {
        throw std::invalid_argument("Некорректный размер буфера кадра");
    }
    pixels_.resize(static_cast<std::size_t>(width) * height);
}

int FrameBuffer::width() const { return width_; }
int FrameBuffer::height() const { return height_; }
int FrameBuffer::pitch() const { return width_ * int(sizeof(std::uint32_t)); }
const std::uint32_t* FrameBuffer::data() const { return pixels_.data(); }

void FrameBuffer::clear(std::uint32_t color) {
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void FrameBuffer::setPixel(int x, int y, std::uint32_t color) {
    if (x >= 0 && y >= 0 && x < width_ && y < height_) {
        pixels_[static_cast<std::size_t>(y) * width_ + x] = color;
    }
}

void FrameBuffer::verticalLine(int x, int begin, int end, std::uint32_t color) {
    if (x < 0 || x >= width_) {
        return;
    }
    begin = std::clamp(begin, 0, height_);
    end = std::clamp(end, 0, height_);
    for (int y = begin; y < end; ++y) {
        pixels_[static_cast<std::size_t>(y) * width_ + x] = color;
    }
}

void FrameBuffer::fillRows(int begin, int end, std::uint32_t color) {
    begin = std::clamp(begin, 0, height_);
    end = std::clamp(end, 0, height_);
    if (begin < end) {
        std::fill(pixels_.begin() + static_cast<std::size_t>(begin) * width_,
                  pixels_.begin() + static_cast<std::size_t>(end) * width_, color);
    }
}

}
