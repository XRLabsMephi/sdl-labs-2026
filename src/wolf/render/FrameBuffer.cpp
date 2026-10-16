#include <iostream>
#include <wolf/render/FrameBuffer.h>

using namespace render;
using std::min;
using std::max;


FrameBuffer::FrameBuffer(int width, int height) : width_(width),
                                                  height_(height) {

    if (width <= 0) throw std::invalid_argument("width must be > 0");
    if (height <= 0) throw std::invalid_argument("height must be > 0");
    buffer_.assign(static_cast<size_t>(width_) * static_cast<size_t>(height_), 0);
}

void FrameBuffer::clear(const DrawCallParams &params) {
    uint32_t color_as_uint = params.color.to_uint32_t();

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

void FrameBuffer::vline(int x, int y0, int y1, const DrawCallParams &params) {
    if (x < 0 || x >= width_) return;
    if (y0 > y1) std::swap(y0, y1);

    y0 = max(y0, 0);
    y1 = min(y1, height_ - 1);
    if (y0 > y1) return;

    uint32_t color_as_uint = params.color.to_uint32_t();

    uint32_t *col = &buffer_[x * height_];
    std::fill_n(col + y0, y1 - y0 + 1, color_as_uint);
    /*for (int y = y0; y <= y1; ++y) {
        col[y] = color_as_uint;
    }*/
}

void FrameBuffer::hline(int y, int x0, int x1, const DrawCallParams &params) {
    if (y < 0 || y >= height_) return;
    if (x0 > x1) std::swap(x0, x1);
    x0 = max(x0, 0);
    x1 = min(x1, width_ - 1);
    if (x0 > x1) return;

    uint32_t color_as_uint = params.color.to_uint32_t();

    const int h = height_;
    uint32_t* p = &buffer_[x0 * height_ + y];
    for (int x = x0; x <= x1; ++x, p += h) {
        *p = color_as_uint;
    }
}

void FrameBuffer::rectangle(int x0, int y0, int x1, int y1, const DrawCallParams &params) {
    if (y0 > y1) std::swap(y0, y1);
    if (x0 > x1) std::swap(x0, x1);

    y0 = max(y0, 0);
    x0 = max(x0, 0);
    x1 = min(x1, width_ - 1);
    y1 = min(y1, height_ - 1);

    if (x0 > x1 || y0 > y1) return;

    uint32_t color_as_uint = params.color.to_uint32_t();

    for (int x = x0; x <= x1; ++x) {
        // TODO Add multithreading
        uint32_t *colPtr = &buffer_[x * height_];
        std::fill_n(colPtr + y0, y1 - y0 + 1, color_as_uint);
        /*for (int y = y0; y <= y1; ++y) {
            colPtr[y] = color_as_uint;
        }*/
    }
}

void FrameBuffer::circle(int x, int y, float radius, const DrawCallParams &params) {
    // TODO
}

void FrameBuffer::triangle(int x0, int y0, int x1, int y1, int x2, int y2, const DrawCallParams &params) {
    // TODO
}
