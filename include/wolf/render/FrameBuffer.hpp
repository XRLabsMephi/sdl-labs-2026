#pragma once

#include "wolf/core/Color.hpp"
#include <vector>

class FrameBuffer {
public:
    FrameBuffer(int width, int height);
    int width() const;
    int height() const;
    void clear(Color color);
    void setPixel(int x, int y, Color color);
    const Color* data() const;

private:
    int width_;
    int height_;
    std::vector<Color> pixels_;
};