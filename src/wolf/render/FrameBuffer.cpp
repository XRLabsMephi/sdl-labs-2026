#include "wolf/render/FrameBuffer.hpp"
#include <algorithm>

namespace wolf::render {

    FrameBuffer::FrameBuffer(int width, int height) : width(width), height(height) {
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
            return;
        }
        int start = std::max(0, std::min(yStart, yEnd));
        int end = std::min(height - 1, std::max(yStart, yEnd));
        std::uint32_t colorValue = color.Shift();
        for (int y = start; y <= end; ++y) {
            pixels[y * width + x] = colorValue;
        }
    }

    //на всяк случай мб нужен будет
    void FrameBuffer::resize(int w, int h) {
        width = std::max(0, w);
        height = std::max(0, h);
        pixels.assign(static_cast<std::size_t>(width) * height, 0);
    }

    const std::uint32_t* FrameBuffer::getPixels() const {
        return pixels.data();
    }
    //return &m_pixels[0]; но на пустом уб => пусть будет как выше

    int FrameBuffer::getWidth() const {
        return width;
    }

    int FrameBuffer::getHeight() const {
        return height;
    }
}