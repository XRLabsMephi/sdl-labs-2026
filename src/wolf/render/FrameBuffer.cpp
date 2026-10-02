#include "render/FrameBuffer.hpp"
#include <stdexcept>

bool FrameBuffer::check_x(size_t x) const {
    return x < width_;
}

bool FrameBuffer::check_y(size_t y) const {
    return y < height_;
}

FrameBuffer::FrameBuffer(size_t width, size_t height): 
width_(width), height_(height), pixels_(new Color[width * height]) {
    clear();
}

FrameBuffer::~FrameBuffer() {
    delete[] pixels_;
}

size_t FrameBuffer::get_width() const {
    return width_;
}

size_t FrameBuffer::get_height() const {
    return height_;
}

Color* FrameBuffer::get_pixels() const {
    return pixels_;
}

void FrameBuffer::set_pixel(size_t x, size_t y, Color color) {
    bool fx = check_x(x);
    bool fy = check_y(y);
    if (fx && fy) {
        pixels_[y * width_ + x] = color;
    }
}

void FrameBuffer::set_color(Color color) {
    for (size_t index = 0; index < width_ * height_; ++index) {
        pixels_[index] = color;
    }
}

void FrameBuffer::clear() {
    set_color(0);
}

void FrameBuffer::draw_vertical_line(size_t x, int y_start, int y_end, Color color) {
    if (!check_x(x)) {
        return;
    }
    if (y_start > y_end) {
        std::swap(y_start, y_end);
    }
    int start = std::max(0, y_start);
    int end = std::min(static_cast<int>(height) - 1, y_end);
    for (int y = start; y <= end; ++y) {
        set_pixel(x, static_cast<size_t>(y), color);
    } 
}

void FrameBuffer::draw_horizontal_line(int x_start, int x_end, size_t y, Color color) {
    if (!check_y(y)) {
        return;
    }
    if (x_start > x_end) {
        std::swap(x_start, x_end);
    }
    int start = std::max(0, x_start);
    int end = std::min(static_cast<int>(width) - 1, x_end);
    for (int x = start; x <= end; ++x) {
        set_pixel(static_cast<size_t>(x), y, color);
    }  
}