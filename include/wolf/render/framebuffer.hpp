#pragma once
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

#include "wolf/core/color.hpp"

namespace render {
    class Framebuffer {
    public:
        static constexpr uint32_t kNoFragment = std::numeric_limits<uint32_t>::max();
        struct TransparentFragment {
            double depth;
            uint32_t color;
            uint32_t next;
        };

    private:
        int width_, height_;
        std::vector<uint32_t> data_;
        std::vector<float> depth_;
        std::vector<uint32_t> transparentHeads_;
        std::vector<TransparentFragment> transparentFragments_;
    public:
        Framebuffer(int width, int height);

        void clear(core::ColorRGBA color = {});

        [[nodiscard]] int width() const noexcept;

        [[nodiscard]] int height() const noexcept;

        [[nodiscard]] uint32_t* color() noexcept;

        [[nodiscard]] const uint32_t* color() const noexcept;

        // Nearest opaque surface depth. Transparent layers keep their own depths.
        // Infinity means background; transparent texels leave depth unchanged.
        [[nodiscard]] float* depth() noexcept;

        [[nodiscard]] const float* depth() const noexcept;

        void setDepth(int x, int y, float depth) noexcept;

        [[nodiscard]] float getDepth(int x, int y) const noexcept;

        void setPixel(int x, int y, core::ColorRGBA color) noexcept;

        [[nodiscard]] uint32_t getPixel(int x, int y) const noexcept;

        // Opaque surfaces update color/depth. Partial alpha is deferred until the
        // final fog/composition pass; alpha zero never writes color or depth.
        void drawFragment(int x, int y, core::ColorRGBA color, double depth);

        [[nodiscard]] const uint32_t* transparentHeads() const noexcept;
        [[nodiscard]] std::span<const TransparentFragment> transparentFragments() const noexcept;
    };
}
