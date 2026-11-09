#pragma once

#include <cmath>

namespace wolf
{

struct Vec2
{
    double x = 0.0;
    double y = 0.0;

    constexpr Vec2 operator+(const Vec2& other) const
    {
        return {x + other.x, y + other.y};
    }

    constexpr Vec2 operator-(const Vec2& other) const
    {
        return {x - other.x, y - other.y};
    }

    constexpr Vec2 operator*(double value) const
    {
        return {x * value, y * value};
    }

    constexpr Vec2& operator+=(const Vec2& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    [[nodiscard]] double length() const
    {
        return std::sqrt(x * x + y * y);
    }

    [[nodiscard]] Vec2 normalized() const
    {
        const double vectorLength = length();
        return vectorLength > 0.0 ? (*this * (1.0 / vectorLength)) : Vec2{};
    }
};

} // namespace wolf
