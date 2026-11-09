#pragma once

namespace wolf
{
struct Vec2
{
    double x = 0.0, y = 0.0;

    Vec2 operator+(const Vec2& other) const
    {
        return {x + other.x, y + other.y};
    }

    Vec2 operator-(const Vec2& other) const
    {
        return {x - other.x, y - other.y};
    }

    Vec2 operator*(double value) const
    {
        return {x * value, y * value};
    }
};
}
