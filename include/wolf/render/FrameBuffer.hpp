#pragma once

#include "wolf/core/Color.hpp"

#include <vector>

namespace wolf
{
class FrameBuffer
{
public:
    FrameBuffer(int width, int height);

    int width() const;
    int height() const;
    const std::vector<Color>& pixels() const;

    void clear(Color color);
    void setPixel(int x, int y, Color color);
    void fillColumn(int x, int y0, int y1, Color color);

private:
    int width_ = 0, height_ = 0;
    std::vector<Color> pixels_;
};
}
