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

        // fill whole buffer with color
        void clear(const core::Color &color);

        // color single pixel at given point (points outside buffer rejects)
        void drawPixel(const Point &point, const Color &color);

        // draws point at given position with given color and size
        void drawPoint(const Point& point, float size, const Color &color);

        // draws line from start to end
        bool drawLine(const Point &start, const Point &end, const Color &color);

        // draws rectangle with left upper corner in start and lower right corner in end
        bool drawRectangle(const Point &start, const Point &end, const Color &color);

        // draws circle of given radius and color
        bool drawCircle(const Point &center, float radius, const Color &color);

        // draws triangle with given vertices
        bool drawTriangle(const Point &p1, const Point &p2, const Point &p3, const Color &color);

        //draws polygon with given vertices
        template<typename... Points>
        void drawPolygon(const Color &color, const Points &... points);
    };
}
