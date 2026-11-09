#include "wolf/core/Color.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SoftwareRenderer.hpp"

#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

template <typename Function>
void requireThrows(Function&& action, const char* message) {
    try { action(); } catch (const std::exception&) { return; }
    throw std::runtime_error(message);
}

void testColorsAndPixels() {
    constexpr wolf::Color color = wolf::makeColor(0x11, 0x22, 0x33, 0x44);
    static_assert(color == 0x11223344u);
    static_assert(wolf::makeColor(1, 2, 3) == 0x010203ffu);
    wolf::FrameBuffer frame(3, 2);
    frame.clear(color);
    for (int y = 0; y < 2; ++y) {
        for (int x = 0; x < 3; ++x) {
            require(frame.pixelAt(x, y) == color, "clear must fill entire frame");
        }
    }
    frame.setPixel(1, 1, wolf::makeColor(7, 8, 9));
    require(frame.pixelAt(1, 1) == wolf::makeColor(7, 8, 9), "setPixel must write");
    frame.setPixel(-1, 0, 0);
    frame.setPixel(3, 0, 0);
    frame.setPixel(0, 2, 0);
    require(frame.pixelAt(0, 0) == color, "out-of-range writes must be ignored");
    requireThrows([&] { (void)frame.pixelAt(-1, 0); }, "invalid reads must throw");
    requireThrows([&] { (void)frame.pixelAt(0, 2); }, "invalid reads must throw");
    requireThrows([] { wolf::FrameBuffer invalid(0, 10); }, "zero dimensions must fail");
    requireThrows([] { wolf::FrameBuffer invalid(-1, 10); }, "negative dimensions must fail");
    require(frame.pixels() != nullptr && frame.width() == 3 && frame.height() == 2,
            "pixel layout and dimensions must be available");
}

void testSpansAndBackground() {
    wolf::FrameBuffer frame(3, 5);
    constexpr wolf::Color c = wolf::makeColor(9, 8, 7);
    frame.clear(0);
    frame.drawVerticalSpan(1, -10, 3, c);
    for (int y = 0; y < 5; ++y) {
        require(frame.pixelAt(1, y) == (y < 3 ? c : 0u), "vertical span must clip");
    }
    frame.drawVerticalSpan(-1, 0, 5, c);
    frame.drawVerticalSpan(3, 0, 5, c);
    frame.drawVerticalSpan(2, 4, 2, c);
    require(frame.pixelAt(2, 4) == 0u, "invalid spans must not write");
    frame.fillRows(4, 999, c);
    for (int x = 0; x < 3; ++x) {
        require(frame.pixelAt(x, 4) == c, "fillRows must clip");
    }
}

void testProjectedWalls() {
    wolf::FrameBuffer frame(4, 6);
    std::vector<wolf::WallSlice> columns(4);
    columns[0].hit.hit = true;
    columns[0].hit.wallId = 1;
    columns[0].hit.side = wolf::WallSide::West;
    columns[0].drawStart = 1;
    columns[0].drawEnd = 5;
    columns[1].hit.hit = true;
    columns[1].hit.wallId = 1;
    columns[1].hit.side = wolf::WallSide::North;
    columns[1].drawStart = -10;
    columns[1].drawEnd = 99;
    columns[2].hit.hit = true;
    columns[2].hit.wallId = 2;
    columns[2].drawStart = 2;
    columns[2].drawEnd = 4;
    wolf::SoftwareRenderer::drawFrame(frame, columns);
    require(frame.pixelAt(3, 0) != frame.pixelAt(3, 5), "ceiling and floor must differ");
    require(frame.pixelAt(0, 0) == frame.pixelAt(3, 0), "above wall is ceiling");
    require(frame.pixelAt(0, 5) == frame.pixelAt(3, 5), "below wall is floor");
    require(frame.pixelAt(0, 2) != frame.pixelAt(1, 2), "faces must be shaded differently");
    require(frame.pixelAt(0, 2) != frame.pixelAt(2, 2), "wall IDs must change color");
    require(frame.pixelAt(1, 0) != frame.pixelAt(3, 0), "out-of-range wall top must clip");
    require(frame.pixelAt(1, 5) != frame.pixelAt(3, 5), "out-of-range wall bottom must clip");
    // Every new draw must replace old pixels, even if the current ray misses.
    wolf::SoftwareRenderer::drawFrame(frame, {});
    require(frame.pixelAt(0, 2) == frame.pixelAt(3, 2), "old wall pixels must be cleared");
    require(frame.pixelAt(0, 5) == frame.pixelAt(3, 5), "floor must remain intact");
}

} // namespace

int main() {
    try {
        testColorsAndPixels();
        testSpansAndBackground();
        testProjectedWalls();
        std::cout << "All SoftwareRenderer tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
