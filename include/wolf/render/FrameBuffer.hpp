#pragma once
#include <vector>
#include <cstdint>
#include "wolf/core/Color.hpp"

namespace wolf::render {
    class FrameBuffer {
    public:
        FrameBuffer(int width, int height);

        void clear(const core::Color& color);

        void setPixel(int x, int y, const core::Color& color);

        void drawVerticalLine(int x, int yStart, int yEnd, const core::Color& color);

        void resize(int w, int h);

        const std::uint32_t* getPixels() const;

        int getWidth() const;

        int getHeight() const;
    private:
        int width;
        int height;
        std::vector<std::uint32_t> pixels;
    };
}