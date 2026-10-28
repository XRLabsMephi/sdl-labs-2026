#include <cstdint>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "wolf/render/FrameBuffer.hpp"

int main() {
    constexpr int W = 32;
    constexpr int H = 24;

    wolf::FrameBuffer frame(W, H);
    const wolf::Color background{20, 30, 40};

    frame.clear(background);

    std::vector<wolf::Color> expected(W * H, background);

    std::mt19937 rng(42);

    std::uniform_int_distribution<int> coords(-40, 80);
    std::uniform_int_distribution<int> sizes(-10, 45);
    std::uniform_int_distribution<int> channel(0, 255);

    for (int iteration = 0; iteration < 500; ++iteration) {
        int x = coords(rng);
        int y = coords(rng);

        wolf::Color color{
            static_cast<std::uint8_t>(channel(rng)),
            static_cast<std::uint8_t>(channel(rng)),
            static_cast<std::uint8_t>(channel(rng))
        };

        if (iteration % 2 == 0) {
            int w = sizes(rng);
            int h = sizes(rng);

            frame.fillRect(x, y, w, h, color);

            if (w > 0 && h > 0) {
                for (int py = 0; py < H; ++py) {
                    for (int px = 0; px < W; ++px) {
                        if (px >= x && 1LL * px < 1LL * x + w &&
                            py >= y && 1LL * py < 1LL * y + h) {
                            expected[py * W + px] = color;
                        }
                    }
                }
            }
        } else {
            int yEnd = coords(rng);

            frame.drawColumn(x, y, yEnd, color);

            for (int py = 0; py < H; ++py) {
                if (x >= 0 && x < W && py >= y && py < yEnd) {
                    expected[py * W + x] = color;
                }
            }
        }

        for (int p = 0; p < W * H; ++p) {
            if (frame.data()[p] != expected[p]) {
                throw std::runtime_error(
                    "Raster mismatch at step " +
                    std::to_string(iteration)
                );
            }
        }
    }

    std::cout << "Rasterization tests passed\n";
}
