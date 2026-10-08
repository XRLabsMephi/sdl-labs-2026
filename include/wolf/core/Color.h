#pragma once
#include <algorithm>
#include <SDL3/SDL_pixels.h>

namespace core {
    struct Color {
        constexpr Color() : r_(0), g_(0), b_(0), a_(0) {
        }

        constexpr Color(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
            : r_(static_cast<Uint8>(r * a / 255))
              , g_(static_cast<Uint8>(g * a / 255))
              , b_(static_cast<Uint8>(b * a / 255))
              , a_(a) {
        }

        static constexpr Color from_premultiplied(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
            Color c;
            c.r_ = r;
            c.g_ = g;
            c.b_ = b;
            c.a_ = a;
            return c;
        }

        Uint8 r() const { return a_ == 0 ? 0 : static_cast<Uint8>((r_ * 255u + a_ / 2) / a_); }
        Uint8 g() const { return a_ == 0 ? 0 : static_cast<Uint8>((g_ * 255u + a_ / 2) / a_); }
        Uint8 b() const { return a_ == 0 ? 0 : static_cast<Uint8>((b_ * 255u + a_ / 2) / a_); }
        Uint8 a() const { return a_; }

        static constexpr Color over(const Color &src, const Color &dst) {
            auto mix = [](Uint8 s, Uint8 d, Uint8 sa) -> Uint8 {
                return static_cast<Uint8>(
                    std::min<int>(255, s + (d * (255 - sa) + 127) / 255));
            };
            return from_premultiplied(
                mix(src.r_, dst.r_, src.a_),
                mix(src.g_, dst.g_, src.a_),
                mix(src.b_, dst.b_, src.a_),
                mix(src.a_, dst.a_, src.a_));
        }

        Color lerp(const Color &other, unsigned percent) const {
            return lerp(*this, other, percent);
        }

        static Color lerp(const Color &lhs, const Color &rhs, unsigned percent) {
            percent = std::min(percent, 100u);
            auto mix = [percent](Uint8 l, Uint8 r) -> Uint8 {
                return static_cast<Uint8>(((100 - percent) * l + percent * r + 50) / 100);
            };
            return from_premultiplied(
                mix(lhs.r_, rhs.r_),
                mix(lhs.g_, rhs.g_),
                mix(lhs.b_, rhs.b_),
                mix(lhs.a_, rhs.a_));
        }

        static constexpr Color add_saturating(const Color &lhs, const Color &rhs) {
            auto add = [](Uint8 l, Uint8 r) -> Uint8 {
                return static_cast<Uint8>(std::min<int>(255, l + r));
            };
            return from_premultiplied(
                add(lhs.r_, rhs.r_),
                add(lhs.g_, rhs.g_),
                add(lhs.b_, rhs.b_),
                add(lhs.a_, rhs.a_));
        }

    private:
        Uint8 r_, g_, b_, a_;
    };
} // namespace core
