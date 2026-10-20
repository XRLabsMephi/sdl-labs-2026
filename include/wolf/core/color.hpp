#pragma once
#include <cstddef>
#include <cstdint>

namespace core {
    struct ColorRGBA {
         union {
            struct {
                uint8_t r;
                uint8_t g;
                uint8_t b;
                uint8_t a;
            };
            uint8_t data[4];
        };

        constexpr ColorRGBA() noexcept : r(0), g(0), b(0), a(255) {}

        constexpr ColorRGBA(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) noexcept
                            : r(red), g(green), b(blue), a(alpha) {}

        [[nodiscard]] constexpr uint8_t& operator[](std::size_t index) noexcept {
            return data[index];
        }

        [[nodiscard]] constexpr const uint8_t& operator[](std::size_t index) const noexcept {
            return data[index];
        }

        [[nodiscard]] constexpr uint32_t pack() const noexcept {
            return packColor(r, g, b, a);
        }

        [[nodiscard]] static constexpr uint32_t packColor(uint8_t red, uint8_t green, 
                                                          uint8_t blue, uint8_t alpha = 255) noexcept {
            return static_cast<uint32_t>(red) | (static_cast<uint32_t>(green) << 8U)
                | (static_cast<uint32_t>(blue) << 16U) | (static_cast<uint32_t>(alpha) << 24U);
        }
    };
}
