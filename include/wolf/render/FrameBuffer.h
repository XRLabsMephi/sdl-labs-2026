#pragma once

#include <vector>

#include "wolf/core/Point.h"
#include "wolf/core/Color.h"

namespace render {
    using Color = core::Color;
    template<typename T>
    using vector = std::vector<T>;

    /**
    * @brief Contains parameters for draw call to @c FrameBuffer.
    */
    struct DrawCallParams {
        int tex_id = -1; /**< ID of texture. -1 is no texture */
        int tex_x = -1; /**< Column in texture. Ignored if tex_id is set to -1*/
        float depth = 0; /**< Depth of the object in the frame (z_buf value). Influences the color*/
        Color color = Color::black; /**< Color is used only if tex_id is -1*/
    };


    /**
     * Pixel buffer for a single frame
     */
    class FrameBuffer {
        int width_; /**< Width of buffer in pixels */
        int height_; /**< Height of buffer in pixels */
        vector<uint32_t> buffer_; /** < flat buffer*/

    public:
        FrameBuffer(int width, int height);

        /**
         * @brief Fills whole buffer
         * @param params draw call parameters (see @c DrawCallParams)
         */
        void clear(const DrawCallParams &params);

        /**
         * @brief Colors single pixel at point
         * @param x x coordinate
         * @param y y coordinate
         * @param params draw call parameters (see @c DrawCallParams)
         * @attention Points outside buffer rejects
         */
        void set(int x, int y, const DrawCallParams &params);

        /**
         * @brief Draws point at given position
         * @param x x coordinate
         * @param y y coordinate
         * @param size point size in pixels
         * @param params draw call parameters (see @c DrawCallParams)
         */
        void point(int x, int y, int size, const DrawCallParams &params);

        /**
         * @brief Draws vertical line
         * @param x x coordinate
         * @param y0 start y coordinate
         * @param y1 end y coordinate
         * @param params draw call parameters (see @c DrawCallParams)
         */
        void vline(int x, int y0, int y1, const DrawCallParams &params);

        /**
         * @brief Draws horizontal line
         * @param y y coordinate
         * @param x0 start x coordinate
         * @param x1 end x coordinate
         * @param params draw call parameters (see @c DrawCallParams)
         */
        void hline(int y, int x0, int x1, const DrawCallParams &params);

        /**
         * @brief Draws rectangle.
         * @param x0 top-left corner x coordinate
         * @param y0 top-left y coordinate
         * @param x1 bottom-right x coordinate
         * @param y1 bottom-right y coordinate
         * @param params draw call parameters (see @c DrawCallParams)
         */
        void rectangle(int x0, int y0, int x1, int y1, const DrawCallParams &params);

        // draws circle of given radius and color
        void circle(int x, int y, float radius, const DrawCallParams &params);

        // draws triangle with given vertices
        bool triangle(int x0, int y0, int x1, int y1, int x2, int y2, const DrawCallParams &params);
    };
}
