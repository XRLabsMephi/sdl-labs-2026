#pragma once

#include <cmath>

namespace core {
    struct Vec2 {
        double x = 0.0;
        double y = 0.0;

        constexpr Vec2() noexcept = default;

        constexpr Vec2(double xVal, double yVal) noexcept : x(xVal), y(yVal) {}

        [[nodiscard]] constexpr Vec2 operator+(const Vec2 &other) const noexcept {
            return {x + other.x, y + other.y};
        }

        [[nodiscard]] constexpr Vec2 operator-(const Vec2 &other) const noexcept {
            return {x - other.x, y - other.y};
        }

        [[nodiscard]] constexpr Vec2 operator*(double scalar) const noexcept {
            return {x * scalar, y * scalar};
        }

        [[nodiscard]] constexpr Vec2 operator/(double scalar) const noexcept {
            return {x / scalar, y / scalar};
        }

        [[nodiscard]] constexpr Vec2 operator-() const noexcept {
            return {-x, -y};
        }

        Vec2 &operator+=(const Vec2 &other) noexcept {
            *this = *this + other;
            return *this;
        }

        Vec2 &operator-=(const Vec2 &other) noexcept {
            *this = *this - other;
            return *this;
        }

        Vec2 &operator*=(double scalar) noexcept {
            *this = *this * scalar;
            return *this;
        }

        [[nodiscard]] constexpr double dot(const Vec2 &other) const noexcept {
            return x * other.x + y * other.y;
        }

        [[nodiscard]] double length() const {
            return std::sqrt(dot(*this));
        }

        [[nodiscard]] Vec2 normalized() const {
            double len = length();
            return len > 0.0 ? *this / len : Vec2{};
        }

        // Перпендикуляр вправо
        [[nodiscard]] constexpr Vec2 perpendicular() const noexcept {
            return {-y, x};
        }

        [[nodiscard]] Vec2 rotated(double angle) const {
            double c = std::cos(angle);
            double s = std::sin(angle);
            return {x * c - y * s, x * s + y * c};
        }
    };

    [[nodiscard]] constexpr Vec2 operator*(double scalar, const Vec2 &v) noexcept {
        return v * scalar;
    }
}
