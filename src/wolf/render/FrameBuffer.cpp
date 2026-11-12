#include "wolf/render/FrameBuffer.hpp"
#include <algorithm>
#include <format>
#include <stdexcept>

namespace wolf::render {

    FrameBuffer::FrameBuffer(int width, int height) : width(width), height(height) {
        if (width <= 0 || height <= 0) {
            throw std::invalid_argument(std::format("FrameBuffer: invalid size {}x{}", width, height));
        }
        pixels.resize(width * height, 0);
    }

    void FrameBuffer::clear(const core::Color& color) {
        std::uint32_t colorValue = color.Shift();
        std::fill(pixels.begin(), pixels.end(), colorValue);
    }

    void FrameBuffer::setPixel(int x, int y, const core::Color& color) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            pixels[y * width + x] = color.Shift();
        }
    }

    void FrameBuffer::drawVerticalLine(int x, int yStart, int yEnd, const core::Color& color) {
        if (x < 0 || x >= width) {
            throw std::runtime_error(std::format("x {} >= width {}", x, width));
        }
        int start = std::max(0, std::min(yStart, yEnd));
        int end = std::min(height - 1, std::max(yStart, yEnd));
        std::uint32_t colorValue = color.Shift();
        for (int y = start; y <= end; ++y) {
            pixels[y * width + x] = colorValue;
        }
    }

    void FrameBuffer::resize(int w, int h) {
        width = std::max(0, w);
        height = std::max(0, h);
        pixels.assign(static_cast<std::size_t>(width) * height, 0);
    }

    [[nodiscard]] const std::uint32_t* FrameBuffer::getPixels() const noexcept {
        return pixels.data();
    }
    //return &m_pixels[0]; но на пустом уб => пусть будет как выше

    [[nodiscard]] int FrameBuffer::getWidth() const noexcept{
        return width;
    }

    [[nodiscard]] int FrameBuffer::getHeight() const noexcept {
        return height;
    }
}
