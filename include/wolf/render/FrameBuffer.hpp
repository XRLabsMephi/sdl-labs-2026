#pragma once

#include <cstdint>
#include <vector>

namespace wolf
{

// Pixels use ARGB8888 (0xAARRGGBB), matching the SDL texture format.
class FrameBuffer
{
public:
    using Pixel = std::uint32_t;

    FrameBuffer(int width, int height);

    [[nodiscard]] int width() const noexcept;
    [[nodiscard]] int height() const noexcept;
    [[nodiscard]] const Pixel* data() const noexcept;
    [[nodiscard]] Pixel* data() noexcept;

    void clear(Pixel color);
    void fillHorizon(Pixel ceilingColor, Pixel floorColor);
    void setPixel(int x, int y, Pixel color);
    void drawVerticalSpan(int x, int firstY, int lastY, Pixel color);

private:
    int width_;
    int height_;
    std::vector<Pixel> pixels_;
};

} // namespace wolf
