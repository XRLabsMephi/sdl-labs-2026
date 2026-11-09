#pragma once

#include "wolf/core/Color.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace bmp_test {

inline void require(bool condition, const char* message) {
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
inline void writeBmp(const std::filesystem::path& path, bool alpha,
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

} // namespace bmp_test
