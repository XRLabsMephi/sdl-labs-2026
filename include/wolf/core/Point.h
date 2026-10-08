#pragma once

#include "Vec2.h"
struct Point {
    float x;
    float y;

    Point operator +(const core::Vector2& vec);
    Point& operator +=(const core::Vector2& vec); //apply offset
    float operator -(const Point& other); // returns distance between points
    Point operator -(const core::Vector2& vec);
    Point& operator -= (const core::Vector2& vec); // apply inverted offset
};
