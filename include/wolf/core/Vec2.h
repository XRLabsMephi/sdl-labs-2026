#pragma once
#include <cmath>

namespace core {
    struct Vector2 {
        double x = 0.0, y = 0.0;

        Vector2 operator-(const Vector2& other) const{
            return {x - other.x, y - other.y};
        }
        Vector2 operator+(const Vector2& other) const
        {
            return {x + other.x, y + other.y};
        }
        Vector2& operator-=(const Vector2& other){
            x -= other.x;
            y -= other.y;
            return *this;
        }
        Vector2& operator+=(const Vector2& other){
            x += other.x;
            y += other.y;
            return *this;
        }
        double length() const{
            return Vector2::length(*this);
        }
        inline static double distance(const Vector2& v1, const Vector2& v2){
            return sqrt((v1.x - v2.x)*(v1.x-v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
        }
        inline static double length(const Vector2& v){
            return sqrt(v.x * v.x + v.y * v.y);
        }

        static double dot(const Vector2& a, const Vector2& b) {
            return a.x * b.x + a.y * b.y;
        }

        double dot(const Vector2& other) const {
            return dot(*this, other);
        }

        static double cross(const Vector2& a, const Vector2& b) {
            return a.x * b.y - a.y * b.x;
        }

        double cross(const Vector2& other) const {
            return cross(*this, other);
        }
    };

}
