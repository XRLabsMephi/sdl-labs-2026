#pragma once
#include <cstdint>
#include <vector>
#include "wolf/core/Point.h"
#include "wolf/core/Color.h"

namespace render {
    using Color = core::Color;
    class FrameBuffer {
        uint32_t width_;
        uint32_t height_;

    public:
        FrameBuffer(int width, int height);

        void clear(const core::Color &color);

        void drawPixel(const Point &point, const Color &color);

        void drawPoint(const Point& point, float size, const Color &color);

        bool drawLine(const Point &start, const Point &end, const Color &color);

        bool drawRectangle(const Point &start, const Point &end, const Color &color);

        bool drawCircle(const Point &center, float radius, const Color &color);

        bool drawTriangle(const Point &p1, const Point &p2, const Point &p3, const Color &color);

        template<typename... Points>
        void drawPolygon(const Color &color, const Points &... points) {
        }
    };
}
