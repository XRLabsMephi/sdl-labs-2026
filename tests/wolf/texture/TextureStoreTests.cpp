#include "wolf/texture/TextureStore.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

template <typename Function>
void requireError(Function&& action, const std::string& text) {
    try {
        action();
    } catch (const std::exception& error) {
        require(std::string(error.what()).find(text) != std::string::npos,
                "Error must identify the resource path or ID");
        return;
    }
    throw std::runtime_error("Expected error: " + text);
}

struct TemporaryDirectory {
    std::filesystem::path path = std::filesystem::temp_directory_path() /
        ("wolf-texture-test-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    TemporaryDirectory() { std::filesystem::create_directory(path); }
    ~TemporaryDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }
};

// An independent BMP fixture writer: 24-bit padded rows or explicit RGBA masks.
void writeBmp(const std::filesystem::path& path, bool alpha,
              const std::array<wolf::Color, 6>& pixels) {
    std::vector<std::uint8_t> bytes;
    const auto u16 = [&](std::uint16_t value) {
        bytes.push_back(static_cast<std::uint8_t>(value));
        bytes.push_back(static_cast<std::uint8_t>(value >> 8));
    };
    const auto u32 = [&](std::uint32_t value) {
        u16(static_cast<std::uint16_t>(value));
        u16(static_cast<std::uint16_t>(value >> 16));
    };
    const std::uint32_t headerSize = alpha ? 56 : 40;
    u16(0x4d42); u32(14 + headerSize + 24); u32(0); u32(14 + headerSize);
    u32(headerSize); u32(3); u32(2); u16(1); u16(alpha ? 32 : 24);
    u32(alpha ? 3 : 0); u32(24); u32(0); u32(0); u32(0); u32(0);
    if (alpha) {
        u32(0x00ff0000); u32(0x0000ff00); u32(0x000000ff); u32(0xff000000);
    }
    for (int y = 1; y >= 0; --y) {
        for (int x = 0; x < 3; ++x) {
            const auto color = pixels[static_cast<std::size_t>(y * 3 + x)];
            bytes.push_back(static_cast<std::uint8_t>(color >> 8));
            bytes.push_back(static_cast<std::uint8_t>(color >> 16));
            bytes.push_back(static_cast<std::uint8_t>(color >> 24));
            if (alpha) { bytes.push_back(static_cast<std::uint8_t>(color)); }
        }
        if (!alpha) { bytes.insert(bytes.end(), 3, 0xcc); }
    }
    std::ofstream file(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    require(static_cast<bool>(file), "Could not write BMP fixture");
}

void testPixelsAndOwnership(const std::filesystem::path& directory) {
    const std::array<wolf::Color, 6> colors{
        0x11223344u, 0xaabbcc00u, 0xff0000ffu,
        0x00ff0080u, 0x0000ffffu, 0xfedcba01u};
    wolf::TextureStore store;
    for (bool alpha : {false, true}) {
        const auto path = directory / (alpha ? "rgba.bmp" : "padded-rgb.bmp");
        writeBmp(path, alpha, colors);
        const int id = alpha ? 7 : 3;
        store.load(id, path);
        std::filesystem::remove(path);
        const auto& texture = store.get(id);
        require(texture.width == 3 && texture.height == 2 && texture.pixels.size() == 6,
                "Loaded dimensions and CPU storage must match BMP");
        for (int y = 0; y < 2; ++y) {
            for (int x = 0; x < 3; ++x) {
                auto expected = colors[static_cast<std::size_t>(y * 3 + x)];
                if (!alpha) { expected |= 0xffu; }
                require(texture.sample(x, y) == expected,
                        "BMP channels, alpha, row order and padded rows must be preserved");
            }
        }
        requireError([&] { (void)texture.sample(-1, 0); }, "coordinates");
        requireError([&] { (void)texture.sample(3, 0); }, "coordinates");
        requireError([&] { (void)texture.sample(0, 2); }, "coordinates");
        requireError([&] { store.load(id, path); }, "Duplicate texture ID");
    }
    require(store.contains(3) && store.contains(7) && !store.contains(99),
            "IDs must identify loaded textures exactly");
    requireError([&] { (void)store.get(99); }, "99");
}

void testFailures(const std::filesystem::path& directory) {
    wolf::TextureStore store;
    const auto missing = directory / "missing.bmp";
    requireError([&] { store.load(1, missing); }, missing.string());
    require(!store.contains(1), "Failed load must not insert a texture");
    const auto corrupt = directory / "corrupt.bmp";
    { std::ofstream file(corrupt); file << "Not a BMP"; }
    requireError([&] { store.load(2, corrupt); }, corrupt.string());
    require(!store.contains(2), "Corrupt image must not insert a texture");
    requireError([&] { store.load(0, missing); }, "positive");
    requireError([&] { store.load(-1, missing); }, "positive");
    const auto truncated = directory / "truncated.bmp";
    writeBmp(truncated, true, {});
    std::filesystem::resize_file(truncated, 72);
    requireError([&] { store.load(4, truncated); }, truncated.string());
    require(!store.contains(4), "Truncated BMP must fail without a partial resource");
}

} // namespace

int main() {
    try {
        const TemporaryDirectory directory;
        testPixelsAndOwnership(directory.path);
        testFailures(directory.path);
        std::cout << "All TextureStore tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
