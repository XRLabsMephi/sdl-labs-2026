#include <cassert>
#include <iostream>

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
    assert(at(1, 1) == red);
    assert(at(0, 0) == gray);

    frame.fillRect(2, 0, 10, 10, blue);
    assert(at(2, 0) == blue);
    assert(at(3, 2) == blue);
    assert(at(1, 1) == red);

    frame.drawColumn(0, -5, 2, red);
    assert(at(0, 0) == red);
    assert(at(0, 1) == red);
    assert(at(0, 2) == gray);

    std::cout << "FrameBuffer tests passed\n";
}
