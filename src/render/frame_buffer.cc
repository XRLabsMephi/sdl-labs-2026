#include "frame_buffer.h"

#include <algorithm>

FrameBuffer::FrameBuffer(int width, int height)
    : width_(width), height_(height), pixels_(width * height, Color::kBlack.GetValue()) {}

void FrameBuffer::Clear(Color ceiling_color, Color floor_color) {
    int half_height = height_ / 2;
    uint32_t ceiling_raw = ceiling_color.GetValue();
    uint32_t floor_raw = floor_color.GetValue();

    for (int y = 0; y < half_height; ++y) {
        for (int x = 0; x < width_; ++x) {
            pixels_[y * width_ + x] = ceiling_raw;
        }
    }

    for (int y = half_height; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            pixels_[y * width_ + x] = floor_raw;
        }
    }
}

void FrameBuffer::SetPixel(int x, int y, Color c) {
    if (x >= 0 && x < width_ && y >= 0 && y < height_) {
        pixels_[y * width_ + x] = c.GetValue();
    }
}

void FrameBuffer::DrawVerticalLine(int x, int start_y, int end_y, Color c) {
    if (x < 0 || x >= width_) return;

    int y0 = std::max(0, start_y);
    int y1 = std::min(height_ - 1, end_y);
    uint32_t raw_color = c.GetValue();

    for (int y = y0; y <= y1; ++y) {
        pixels_[y * width_ + x] = raw_color;
    }
}

int FrameBuffer::GetWidth() const { return width_; }

int FrameBuffer::GetHeight() const { return height_; }

const uint32_t* FrameBuffer::GetPixelData() const {
    return pixels_.data();
}