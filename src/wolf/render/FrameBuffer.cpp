#include <render/FrameBuffer.h>
#include <stdexcept>
#include <ranges>
#include <algorithm>
#include <cstdint>
#include <limits>

using namespace render;
using std::min;
using std::max;


FrameBuffer::FrameBuffer(int width, int height) : width_(0), height_(0), int_width_(0), int_height_(0) {
    if (width <= 0) { throw std::invalid_argument("width must be > 0"); }
    if (height <= 0) { throw std::invalid_argument("height must be > 0"); }

    width_ = static_cast<buf_size_t>(width);
    height_ = static_cast<buf_size_t>(height);

    if (width_ > std::numeric_limits<buf_size_t>::max() / height_) {
        throw std::length_error("Buffer dimensions are too large");
    }

    int_width_ = width;
    int_height_ = height;

    buffer_.assign(width_ * height_, 0);
}

buf_size_t FrameBuffer::width() const { return width_; }

buf_size_t FrameBuffer::height() const { return height_; }

const uint32_t *FrameBuffer::raw_data() const { return buffer_.data(); }


void FrameBuffer::clear(const DrawCallParams &params) {
    const uint32_t color_as_uint = params.color.to_uint32_t();

    std::ranges::fill(buffer_, color_as_uint);
    /*
     const int w = width_;
     const int h = height_;
     for (int x = 0; x < w; ++x) {
        // TODO Add multithreading
        uint32_t *colPtr = &buffer_[x * h];
        for (int y = 0; y < h; ++y) {
            colPtr[y] = color_as_uint;
        }
    }*/
}

void FrameBuffer::set(int x, int y, const DrawCallParams &params) {
    if (x < 0 || y < 0 || x >= int_width_ || y >= int_height_) return;

    const uint32_t color_as_uint = params.color.to_uint32_t();

    buffer_[x * height_ + y] = color_as_uint;
}

void FrameBuffer::vline(int x, int y0, int y1, const DrawCallParams &params) {
    if (x < 0 || x >= int_width_) return;
    if (y0 > y1) std::swap(y0, y1);

    y0 = max(y0, 0);
    y1 = min(y1, int_height_ - 1);
    if (y0 > y1) return;

    const uint32_t color_as_uint = params.color.to_uint32_t();

    uint32_t *col = &buffer_[x * height_];
    std::fill_n(col + y0, y1 - y0 + 1, color_as_uint);
    /*for (int y = y0; y <= y1; ++y) {
        col[y] = color_as_uint;
    }*/
}

void FrameBuffer::hline(const int y, int x0, int x1, const DrawCallParams &params) {
    if (y < 0 || y >= int_height_) return;
    if (x0 > x1) std::swap(x0, x1);
    x0 = max(x0, 0);
    x1 = min(x1, int_width_ - 1);
    if (x0 > x1) return;

    const uint32_t color_as_uint = params.color.to_uint32_t();

    const buf_size_t h = height_;
    const auto x_end = static_cast<buf_size_t>(x1);
    for (buf_size_t x = x0; x <= x_end; ++x) {
        buffer_[x * h + y] = color_as_uint;
    }
}

void FrameBuffer::rectangle(int x0, int y0, int x1, int y1, const DrawCallParams &params) {
    if (y0 > y1) std::swap(y0, y1);
    if (x0 > x1) std::swap(x0, x1);

    y0 = max(y0, 0);
    x0 = max(x0, 0);
    x1 = min(x1, int_width_ - 1);
    y1 = min(y1, int_height_ - 1);

    if (x0 > x1 || y0 > y1) return;

    const uint32_t color_as_uint = params.color.to_uint32_t();

    for (int x = x0; x <= x1; ++x) {
        // TODO Add multithreading
        uint32_t *colPtr = &buffer_[x * height_];
        std::fill_n(colPtr + y0, y1 - y0 + 1, color_as_uint);
        /*for (int y = y0; y <= y1; ++y) {
            colPtr[y] = color_as_uint;
        }*/
    }
}

void FrameBuffer::circle(int x, int y, double radius, const DrawCallParams &params) {
    // TODO
}

void FrameBuffer::triangle(int x0, int y0, int x1, int y1, int x2, int y2, const DrawCallParams &params) {
    // TODO
}
