#pragma once
#include <cstdint>
#include <vector>

#include "wolf/core/color.hpp"

namespace render {
    class Framebuffer {
    private:
        int width_, height_;
        std::vector<uint32_t> data_;
        std::vector<float> depth_;
    public:
        Framebuffer(int width, int height);

        void clear(core::ColorRGBA color = {});

        [[nodiscard]] int width() const noexcept;

        [[nodiscard]] int height() const noexcept;

        [[nodiscard]] uint32_t* color() noexcept;

        [[nodiscard]] const uint32_t* color() const noexcept;

        [[nodiscard]] float* depth() noexcept;

        [[nodiscard]] const float* depth() const noexcept;

        void setDepth(int x, int y, float depth) noexcept;

        [[nodiscard]] float getDepth(int x, int y) const noexcept;

        void setPixel(int x, int y, core::ColorRGBA color) noexcept;

        [[nodiscard]] uint32_t getPixel(int x, int y) const noexcept;
    };
}
