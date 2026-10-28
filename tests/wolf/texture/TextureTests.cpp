#include "wolf/texture/Texture.hpp"

#include <iostream>
#include <stdexcept>

namespace {

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

void requireOutOfRange(const wolf::Texture& texture, int x, int y) {
    try { (void)texture.sample(x, y); }
    catch (const std::out_of_range&) { return; }
    throw std::runtime_error("Invalid texture access must throw std::out_of_range");
}

void testSampling() {
    const wolf::Texture texture{2, 2, {0x11223344u, 0x55667700u, 0x8899aaffu, 0xbbccddee}};
    require(texture.sample(0, 0) == 0x11223344u && texture.sample(1, 0) == 0x55667700u &&
                texture.sample(0, 1) == 0x8899aaffu && texture.sample(1, 1) == 0xbbccddeeu,
            "Sampling must preserve packed channels and row-major layout");
    requireOutOfRange(texture, -1, 0);
    requireOutOfRange(texture, 0, -1);
    requireOutOfRange(texture, 2, 0);
    requireOutOfRange(texture, 0, 2);
    requireOutOfRange({}, 0, 0);
    requireOutOfRange({-1, 2, {}}, 0, 0);
    requireOutOfRange({2, 2, {1, 2, 3}}, 1, 1);
}

} // namespace

int main() {
    try {
        testSampling();
        std::cout << "All Texture tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
