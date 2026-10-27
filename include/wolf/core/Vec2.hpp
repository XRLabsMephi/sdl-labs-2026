#pragma once

namespace wolf {

    struct Vec2 {
        double x = 0.0;
        double y = 0.0;

        [[nodiscard]] constexpr Vec2 operator+(Vec2 other) const noexcept {
            return {x + other.x, y + other.y};
        }

        [[nodiscard]] constexpr Vec2 operator*(double factor) const noexcept {
            return {x * factor, y * factor};
        }
    };

}
