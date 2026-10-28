#include "wolf/texture/BmpLoader.hpp"
#include "BmpFixture.hpp"

#include <iostream>

namespace {

using namespace bmp_test;

void testPixels(const std::filesystem::path& directory) {
    const std::array<wolf::Color, 6> colors{
        0x11223344u, 0xaabbcc00u, 0xff0000ffu,
        0x00ff0080u, 0x0000ffffu, 0xfedcba01u};
    for (bool alpha : {false, true}) {
        const auto path = directory / (alpha ? "rgba.bmp" : "padded-rgb.bmp");
        writeBmp(path, alpha, colors);
        int width = 0, height = 0;
        const auto pixels = wolf::loadBmpPixels(path, width, height);
        std::filesystem::remove(path);
        require(width == 3 && height == 2 && pixels.size() == 6,
                "Loaded dimensions and owned CPU pixels must match BMP");
        for (std::size_t i = 0; i < colors.size(); ++i) {
            const auto expected = alpha ? colors[i] : colors[i] | 0xffu;
            require(pixels[i] == expected,
                    "BMP channels, alpha, row order and padded rows must be preserved");
        }
    }
}

void testErrors(const std::filesystem::path& directory) {
    int width = 19, height = 23;
    const auto missing = directory / "missing.bmp";
    requireError([&] { (void)wolf::loadBmpPixels(missing, width, height); }, missing.string());
    const auto corrupt = directory / "corrupt.bmp";
    { std::ofstream file(corrupt); file << "Not a BMP"; }
    requireError([&] { (void)wolf::loadBmpPixels(corrupt, width, height); }, corrupt.string());
    const auto truncated = directory / "truncated.bmp";
    writeBmp(truncated, true, {});
    std::filesystem::resize_file(truncated, 72);
    requireError([&] { (void)wolf::loadBmpPixels(truncated, width, height); }, truncated.string());
    require(width == 19 && height == 23, "Failed loading must preserve output dimensions");
}

} // namespace

int main() {
    try {
        const TemporaryDirectory directory;
        testPixels(directory.path);
        testErrors(directory.path);
        std::cout << "All BMP loader tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
