#include <iostream>
#include <limits>
#include <stdexcept>

#include "wolf/render/FrameBuffer.hpp"

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

int main() {
    using wolf::Color;
    using wolf::FrameBuffer;

    FrameBuffer frame(4, 3);

    Color gray{80, 80, 80};
    Color red{255, 0, 0};
    Color blue{0, 0, 255};

    auto at = [&](int x, int y) {
        return frame.data()[y * frame.width() + x];
    };

    frame.clear(gray);
    check(at(0, 0) == gray, "clear: first pixel");
    check(at(3, 2) == gray, "clear: last pixel");

    frame.setPixel(1, 1, red);
    frame.setPixel(-1, 0, red);
    frame.setPixel(4, 0, red);
    frame.setPixel(0, 3, red);

    check(at(1, 1) == red, "setPixel: valid pixel");
    check(at(0, 0) == gray, "setPixel: clipping");

    frame.fillRect(-2, -2, 3, 3, blue);

    check(at(0, 0) == blue, "fillRect: negative coordinates");
    check(at(1, 0) == gray, "fillRect: clipping");
    check(at(1, 1) == red, "fillRect: unrelated pixel");

    frame.fillRect(2, 0, 10, 10, blue);

    check(at(2, 0) == blue, "fillRect: top edge");
    check(at(3, 2) == blue, "fillRect: bottom edge");
    check(at(1, 1) == red, "fillRect: preserved pixel");

    frame.drawColumn(0, -5, 2, red);

    check(at(0, 0) == red, "drawColumn: first pixel");
    check(at(0, 1) == red, "drawColumn: second pixel");
    check(at(0, 2) == gray, "drawColumn: exclusive end");

    frame.drawColumn(3, 1, 100, red);

    check(at(3, 0) == blue, "drawColumn: upper boundary");
    check(at(3, 1) == red, "drawColumn: middle pixel");
    check(at(3, 2) == red, "drawColumn: lower boundary");

    frame.drawColumn(1, 2, 2, red);
    frame.drawColumn(-1, 0, 3, red);
    frame.drawColumn(4, 0, 3, red);

    frame.fillRect(5, 0, 3, 3, red);
    frame.fillRect(1, 2, 0, 1, red);
    frame.fillRect(1, 2, -1, 1, red);

    check(at(1, 2) == gray, "empty or outside drawings");

    constexpr int MAX = std::numeric_limits<int>::max();

    frame.fillRect(MAX, 0, MAX, 1, red);
    frame.fillRect(0, MAX, 1, MAX, red);

    check(at(1, 2) == gray, "extreme coordinates");

    bool rejected = false;

    try {
        FrameBuffer invalid(0, 3);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }

    check(rejected, "zero width must be rejected");

    rejected = false;

    try {
        FrameBuffer invalid(4, -1);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }

    check(rejected, "negative height must be rejected");

    std::cout << "FrameBuffer tests passed\n";
}
