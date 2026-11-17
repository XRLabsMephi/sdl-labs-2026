#pragma once

#include <vector>
#include "wolf/core/Color.hpp"

namespace wolf {

class FrameBuffer {
public:
    FrameBuffer(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }

    void clear(Color color);
    void setPixel(int x, int y, Color color);

    void fillRect(int x, int y, int w, int h, Color color);
    void drawColumn(int x, int yBegin, int yEnd, Color color);

    const Color* data() const { return pixels_.data(); }

private:
    int width_, height_;
    std::vector<Color> pixels_;
};

}