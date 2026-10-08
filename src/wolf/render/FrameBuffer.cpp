#include "render/FrameBuffer.hpp"
#include <stdexcept>

bool FrameBuffer::check_x(int x) const {
    return 0 <= x && x < width_;
}

bool FrameBuffer::check_y(int y) const {
    return 0 <= y && y < height_;
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

void FrameBuffer::set_pixel(int x, int y, Color color) {
    if (check_x(x) && check_y(y)) {
        pixels_[static_cast<size_t>(y) * width_ + static_cast<size_t>(x)] = color;
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

void FrameBuffer::draw_vertical_line(int x, int y_start, int y_end, Color color) {
    if (!check_x(x)) {
        return;
    }
    if (y_start > y_end) {
        std::swap(y_start, y_end);
    }
    int start = std::max(0, y_start);
    int end = std::min(static_cast<int>(height_) - 1, y_end);
    for (int y = start; y <= end; ++y) {
        set_pixel(x, y, color);
    } 
}

void FrameBuffer::draw_horizontal_line(int x_start, int x_end, int y, Color color) {
    if (!check_y(y)) {
        return;
    }
    if (x_start > x_end) {
        std::swap(x_start, x_end);
    }
    int start = std::max(0, x_start);
    int end = std::min(static_cast<int>(width_) - 1, x_end);
    for (int x = start; x <= end; ++x) {
        set_pixel(x, y, color);
    }  
}