#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "wolf/render/FrameBuffer.hpp"

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
    assert(at(0, 0) == gray);
    assert(at(3, 2) == gray);

    frame.setPixel(1, 1, red);
    frame.setPixel(-1, 0, red);
    frame.setPixel(4, 0, red);
    frame.setPixel(0, 3, red);
    assert(at(1, 1) == red);
    assert(at(0, 0) == gray);

    frame.fillRect(-2, -2, 3, 3, blue);
    assert(at(0, 0) == blue);
    assert(at(1, 0) == gray);
    assert(at(1, 1) == red);

    frame.fillRect(2, 0, 10, 10, blue);
    assert(at(2, 0) == blue);
    assert(at(3, 2) == blue);
    assert(at(1, 1) == red);

    frame.drawColumn(0, -5, 2, red);
    assert(at(0, 0) == red);
    assert(at(0, 1) == red);
    assert(at(0, 2) == gray);

    frame.drawColumn(3, 1, 100, red);
    assert(at(3, 0) == blue);
    assert(at(3, 1) == red);
    assert(at(3, 2) == red);

    frame.drawColumn(1, 2, 2, red);
    frame.drawColumn(-1, 0, 3, red);
    frame.drawColumn(4, 0, 3, red);
    frame.fillRect(5, 0, 3, 3, red);
    frame.fillRect(1, 2, 0, 1, red);
    frame.fillRect(1, 2, -1, 1, red);
    assert(at(1, 2) == gray);

    constexpr int MAX = std::numeric_limits<int>::max();
    frame.fillRect(MAX, 0, MAX, 1, red);
    frame.fillRect(0, MAX, 1, MAX, red);
    assert(at(1, 2) == gray);

    bool rejected = false;
    try {
        FrameBuffer invalid(0, 3);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected);

    rejected = false;
    try {
        FrameBuffer invalid(4, -1);
    } catch (const std::invalid_argument&) {
        rejected = true;
    }
    assert(rejected);

    std::cout << "FrameBuffer tests passed\n";
}
