#include "wolf/render/FrameBuffer.hpp"

#include <algorithm>
#include <cstddef>

namespace wolf
{
FrameBuffer::FrameBuffer(int width, int height) : width_(width), height_(height), pixels_(static_cast<std::size_t>(width) * height, 0)
{
}

int FrameBuffer::width() const
{
    return width_;
}

int FrameBuffer::height() const
{
    return height_;
}

const std::vector<Color>& FrameBuffer::pixels() const
{
    return pixels_;
}

void FrameBuffer::clear(Color color)
{
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void FrameBuffer::setPixel(int x, int y, Color color)
{
    if (x < 0 || y < 0 || x >= width_ || y >= height_)
    {
        return;
    }

    pixels_[static_cast<std::size_t>(y) * width_ + x] = color;
}

void FrameBuffer::fillColumn(int x, int y0, int y1, Color color)
{
    if (x < 0 || x >= width_)
    {
        return;
    }

    y0 = std::clamp(y0, 0, height_ - 1);
    y1 = std::clamp(y1, 0, height_ - 1);

    if (y0 > y1)
    {
        std::swap(y0, y1);
    }

    for (int y = y0; y <= y1; ++y)
    {
        setPixel(x, y, color);
    }
}
}
