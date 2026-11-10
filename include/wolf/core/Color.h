#pragma once
#include <SDL3/SDL_pixels.h>
#include <array>

namespace core {
    constexpr auto make_dim_lut() {
        std::array<std::array<Uint8, 256>, 256> t{};
        for (int b = 0; b < 256; ++b)
            for (int v = 0; v < 256; ++v)
                t[b][v] = static_cast<Uint8>((v * b + 127) / 255);
        return t;
    }

    inline constexpr auto DIM = make_dim_lut();

    /**
     * @brief RGBA color stored as a single packed 0xAARRGGBB
     * Components are private; use r()/g()/b()/a() to read and
     * setR()/setG()/setB()/setA() to write. The struct is exactly
     */
    struct Color {
    private:
        Uint32 u_col_; /**< Packed color, layout 0xAARRGGBB */

        [[nodiscard]] static constexpr Uint32 pack(Uint8 r, Uint8 g, Uint8 b, Uint8 a) noexcept {
            return static_cast<Uint32>(a) << 24
                   | static_cast<Uint32>(r) << 16
                   | static_cast<Uint32>(g) << 8
                   | static_cast<Uint32>(b);
        }

    public:
        /** @brief Constructs fully transparent black (0x00000000). */
        constexpr Color() noexcept : u_col_(0) {
        }

        /**
         * @brief Constructs a color from individual components.
         * @param r red component, range [0, 255]
         * @param g green component, range [0, 255]
         * @param b blue component, range [0, 255]
         * @param a alpha component, range [0, 255], defaults to 255 (opaque)
         */
        constexpr Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) noexcept
            : u_col_(pack(r, g, b, a)) {
        }

        /** @brief Returns the red component, range [0, 255]. */
        [[nodiscard]] constexpr Uint8 r() const noexcept { return static_cast<Uint8>((u_col_ >> 16) & 0xFFu); }
        /** @brief Returns the green component, range [0, 255]. */
        [[nodiscard]] constexpr Uint8 g() const noexcept { return static_cast<Uint8>((u_col_ >> 8) & 0xFFu); }
        /** @brief Returns the blue component, range [0, 255]. */
        [[nodiscard]] constexpr Uint8 b() const noexcept { return static_cast<Uint8>(u_col_ & 0xFFu); }
        /** @brief Returns the alpha component, range [0, 255]. */
        [[nodiscard]] constexpr Uint8 a() const noexcept { return static_cast<Uint8>((u_col_ >> 24) & 0xFFu); }

        /** @brief Sets the red component, range [0, 255]. */
        constexpr void setR(Uint8 v) noexcept { u_col_ = (u_col_ & 0xFF00FFFFu) | (static_cast<Uint32>(v) << 16); }
        /** @brief Sets the green component, range [0, 255]. */
        constexpr void setG(Uint8 v) noexcept { u_col_ = (u_col_ & 0xFFFF00FFu) | (static_cast<Uint32>(v) << 8); }
        /** @brief Sets the blue component, range [0, 255]. */
        constexpr void setB(Uint8 v) noexcept { u_col_ = (u_col_ & 0xFFFFFF00u) | static_cast<Uint32>(v); }
        /** @brief Sets the alpha component, range [0, 255]. */
        constexpr void setA(Uint8 v) noexcept { u_col_ = (u_col_ & 0x00FFFFFFu) | (static_cast<Uint32>(v) << 24); }

        /**
         * @brief Converts the color into a single 32-bit number (0xAARRGGBB).
         * @return Uint32 representation of the color
         */
        [[nodiscard]] explicit constexpr operator Uint32() const noexcept { return u_col_; }

        /**
         * @brief Constructs a Color from a packed 0xAARRGGBB value.
         * @param bits packed representation
         * @return Color with the given packed representation
         */
        [[nodiscard]] static constexpr Color fromBits(Uint32 bits) noexcept {
            Color c;
            c.u_col_ = bits;
            return c;
        }

        /**
         * @brief Linear interpolation between this color and the given one.
         * @param other color to blend into current
         * @param percent [0, 255] amount of transition between current and other
         * @return resulting Color
         */
        [[nodiscard]] constexpr Color lerp(const Color &other, Uint8 percent) const noexcept {
            return lerp(*this, other, percent);
        }

        /**
         * @brief Linear interpolation between two given colors.
         * @param lhs first color
         * @param rhs second color
         * @param percent [0, 255] amount of transition between lhs and rhs
         * @return resulting Color
         */
        [[nodiscard]] static constexpr Color lerp(const Color &lhs, const Color &rhs, Uint8 percent) noexcept {
            const Uint32 t = percent;
            const Uint32 inv = 255u - t;
            auto mix = [inv, t](const Uint8 l, const Uint8 r) -> Uint8 {
                return static_cast<Uint8>((inv * l + t * r + 127u) / 255u);
            };
            return {
                mix(lhs.r(), rhs.r()),
                mix(lhs.g(), rhs.g()),
                mix(lhs.b(), rhs.b()),
                mix(lhs.a(), rhs.a())
            };
        }

        /**
         * @brief Multiplies R, G, B components by scalar and clamps to [0, 255].
         * @param scalar multiplier, 255 = 1.0 (no change), 0 = black; alpha is preserved
         * @return reference to updated itself (for chaining)
         */
        constexpr Color &multiplyColorsByScalar(const Uint8 scalar) noexcept {
            u_col_ = pack(DIM[scalar][r()], DIM[scalar][g()], DIM[scalar][b()], a());
            return *this;
        }

        /**
         * @brief Multiplies R, G, B components of color by scalar and returns the result.
         * @param color the color to apply operation to
         * @param scalar multiplier, 255 = 1.0 (no change), 0 = black; alpha is preserved
         * @return the result of multiplication
         */
        [[nodiscard]] static constexpr Color multiplyColorsByScalar(const Color &color, Uint8 scalar) noexcept {
            return Color{
                DIM[scalar][color.r()],
                DIM[scalar][color.g()],
                DIM[scalar][color.b()],
                color.a()
            };
        }

        /**
         * @brief Source-over blend of this color onto dst.
         *
         * This color is the source, dst is the destination. Assumes dst is
         * fully opaque (dst.a == 255) — the invariant of the frame buffer.
         * The result is always opaque (0xFF alpha).
         *
         * @param dst background color, must be opaque
         * @return resulting Color
         */
        [[nodiscard]] constexpr Color over(const Color &dst) const noexcept {
            const Uint32 sa = a();
            if (sa == 0) return dst;
            if (sa == 255) return *this;
            const auto *da = DIM[sa].data();
            const auto *dia = DIM[255 - sa].data();
            const Uint32 rr = da[r()] + dia[dst.r()];
            const Uint32 gg = da[g()] + dia[dst.g()];
            const Uint32 bb = da[b()] + dia[dst.b()];
            return fromBits(0xFF000000u | (rr << 16) | (gg << 8) | bb);
        }

        [[nodiscard]] constexpr Color dimmed(const Uint8 *lut) const noexcept {
            return fromBits((u_col_ & 0xFF000000u)
                            | (static_cast<Uint32>(lut[r()]) << 16)
                            | (static_cast<Uint32>(lut[g()]) << 8)
                            | static_cast<Uint32>(lut[b()]));
        }

        static const Color red;
        static const Color green;
        static const Color blue;
        static const Color black;
        static const Color white;
    };

    inline const Color Color::red{255, 0, 0, 255};
    inline const Color Color::green{0, 255, 0, 255};
    inline const Color Color::blue{0, 0, 255, 255};
    inline const Color Color::black{0, 0, 0, 255};
    inline const Color Color::white{255, 255, 255, 255};
} // namespace core
