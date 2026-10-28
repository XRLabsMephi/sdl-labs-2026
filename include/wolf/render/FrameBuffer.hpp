#pragma once

#include "wolf/core/Color.hpp"

#include <vector>

namespace wolf {

class FrameBuffer {
public:
    FrameBuffer(int width, int height);

    [[nodiscard]] int width() const noexcept;
    [[nodiscard]] int height() const noexcept;
    [[nodiscard]] const Color* pixels() const noexcept;

    void clear(Color color) noexcept;
    void fillRows(int firstRow, int endRow, Color color) noexcept;
    void setPixel(int x, int y, Color color) noexcept;
    void drawVerticalSpan(int x, int firstRow, int endRow, Color color) noexcept;

    // Valid coordinates only: throws std::out_of_range for invalid access.
    [[nodiscard]] Color pixelAt(int x, int y) const;

private:
    [[nodiscard]] bool inBounds(int x, int y) const noexcept;

    int width_;
    int height_;
    std::vector<Color> pixels_;
};

} // namespace wolf
