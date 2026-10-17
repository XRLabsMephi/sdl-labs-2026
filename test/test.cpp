#include "wolf/render/FrameBuffer.hpp"

#include <iostream>
#include <stdexcept>

int main() {
    FrameBuffer frame(3, 2);
    Color red = makeColor(255, 0, 0);
    Color green = makeColor(0, 255, 0);
    frame.clear(red);
    frame.setPixel(1, 1, green);
    frame.setPixel(-1, 0, green);
    frame.setPixel(3, 0, green);
    frame.setPixel(0, -1, green);
    frame.setPixel(0, 2, green);
    std::cout << frame.width() << ' ' << frame.height() << '\n';
    for (int y = 0; y < frame.height(); ++y) {
        for (int x = 0; x < frame.width(); ++x) {
            std::cout << std::hex<< frame.data()[y * frame.width() + x]<< ' ';
        }
        std::cout << '\n';
    }
    try {
        FrameBuffer invalid(0, 2);
        std::cout << "ERROR: invalid size accepted\n";
        return 1;
    } catch (const std::invalid_argument &) {
        std::cout << "Invalid size rejected\n";
    }
    return 0;
}
