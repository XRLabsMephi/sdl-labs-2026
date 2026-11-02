#pragma once

#include <cstdint>
#include <vector>

namespace wolf {

class FrameBuffer {
public:
    FrameBuffer(int width, int height);

    int width() const;
    int height() const;
    int pitch() const;
    const std::uint32_t* data() const;

    void clear(std::uint32_t color);
    void setPixel(int x, int y, std::uint32_t color);
    void verticalLine(int x, int begin, int end, std::uint32_t color);
    void fillRows(int begin, int end, std::uint32_t color);

private:
    int width_;
    int height_;
    std::vector<std::uint32_t> pixels_;
};

}
