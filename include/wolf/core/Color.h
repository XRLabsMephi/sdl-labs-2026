#pragma once
#include <SDL3/SDL_pixels.h>
#include <algorithm>

namespace core {
    struct Color {
        Uint8 r_; /**< R component, range [0, 255]*/
        Uint8 g_; /**< G component, range [0, 255]*/
        Uint8 b_; /**< B component, range [0, 255]*/
        Uint8 a_; /**< A component, range [0, 255]*/

        constexpr Color() : r_(0), g_(0), b_(0), a_(0) {}

        constexpr Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) : r_(r), g_(g), b_(b), a_(a) {}

        /**
         * @brief Converts current color into single 32-bit number
         * @return uint32_t representation of color
         */
        uint32_t to_uint32_t() const {
            return to_uint32_t(*this);
        }

        /**
         * @brief Converts given color into single 32-bit number
         * @return uint32_t representation of color
         */
        static uint32_t to_uint32_t(const Color &color) {
            return color.r_ << 24 | color.g_ << 16 | color.b_ << 8 | color.a_;
        }

        /**
         * @brief Converts single 32-bit number into color
         * @return Color from this 32-bit number
         */
        static Color from_uint32_t(uint32_t rgba) {
            return {
                static_cast<Uint8>(rgba >> 24),
                static_cast<Uint8>(rgba >> 16 & 255),
                static_cast<Uint8>(rgba >> 8 & 255),
                static_cast<Uint8>(rgba & 255)
            };
        }

        /**
         * @brief Linear interpolation between this color and given one
         * @param other color to blend into current
         * @param percent [0, 100] percentage of transition between current and other.
         * @return resulting Color
         */
        Color lerp(const Color &other, unsigned percent) const {
            return lerp(*this, other, percent);
        }

        /**
         * @brief Linear interpolation between 2 given colors
         * @param lhs first color
         * @param rhs second color
         * @param percent [0, 100] percentage of transition between lhs and rhs.
         * @return resulting Color
         */
        static Color lerp(const Color &lhs, const Color &rhs, unsigned percent) {
            percent = std::min(percent, 100u);
            auto mix = [percent](Uint8 l, Uint8 r) -> Uint8 {
                return static_cast<Uint8>(((100 - percent) * l + percent * r + 50) / 100);
            };
            return {
                mix(lhs.r_, rhs.r_),
                mix(lhs.g_, rhs.g_),
                mix(lhs.b_, rhs.b_),
                mix(lhs.a_, rhs.a_)
            };
        }

        static const Color red;
        static const Color green;
        static const Color blue;
        static const Color black;
        static const Color white;
    };

    inline const Color Color::red{255, 0, 0, 255};          /**< Preset for red color*/
    inline const Color Color::green{0, 255, 0, 255};        /**< Preset for green color*/
    inline const Color Color::blue{0, 0, 255, 255};         /**< Preset for blue color*/
    inline const Color Color::black{0, 0, 0, 255};          /**< Preset for black color*/
    inline const Color Color::white{255, 255, 255, 255};    /**< Preset for white color*/
} // namespace core
