#include "wolf/render/FrameBuffer.hpp"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace wolf
{

FrameBuffer::FrameBuffer(int width, int height)
    : width_(width)
    , height_(height)
{
    if (width <= 0 || height <= 0 || width > std::numeric_limits<int>::max() / static_cast<int>(sizeof(Pixel)))
    {
        throw std::invalid_argument("Invalid frame buffer dimensions");
    }

    const auto rowCount = static_cast<std::size_t>(height);
    if (static_cast<std::size_t>(width) > pixels_.max_size() / rowCount)
    {
        throw std::length_error("Frame buffer is too large");
    }
    pixels_.resize(static_cast<std::size_t>(width) * rowCount);
}

int FrameBuffer::width() const noexcept
{
    return width_;
}

int FrameBuffer::height() const noexcept
{
    return height_;
}

const FrameBuffer::Pixel* FrameBuffer::data() const noexcept
{
    return pixels_.data();
}

FrameBuffer::Pixel* FrameBuffer::data() noexcept
{
    return pixels_.data();
}

void FrameBuffer::clear(Pixel color)
{
    std::fill(pixels_.begin(), pixels_.end(), color);
}

void FrameBuffer::fillHorizon(Pixel ceilingColor, Pixel floorColor)
{
    const auto middle = static_cast<std::size_t>(height_ / 2) * static_cast<std::size_t>(width_);
    std::fill(pixels_.begin(), pixels_.begin() + static_cast<std::ptrdiff_t>(middle), ceilingColor);
    std::fill(pixels_.begin() + static_cast<std::ptrdiff_t>(middle), pixels_.end(), floorColor);
}

void FrameBuffer::setPixel(int x, int y, Pixel color)
{
    if (x < 0 || x >= width_ || y < 0 || y >= height_)
    {
        return;
    }
    pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)] = color;
}

void FrameBuffer::drawVerticalSpan(int x, int firstY, int lastY, Pixel color)
{
    if (x < 0 || x >= width_ || firstY > lastY)
    {
        return;
    }

    const int top = std::max(firstY, 0);
    const int bottom = std::min(lastY, height_ - 1);
    for (int y = top; y <= bottom; ++y)
    {
        pixels_[static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x)] = color;
    }
}

} // namespace wolf
