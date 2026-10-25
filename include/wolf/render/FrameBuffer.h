#pragma once

#include <vector>
#include "core/Color.h"

namespace render {
    using Color = core::Color;
    template<typename T>
    using vector = std::vector<T>;
    using buf_size_t = size_t;

    /**
    * @brief Contains parameters for draw call to @c FrameBuffer.
    */
    struct DrawCallParams {
        int tex_id = -1; /**< ID of texture. -1 is no texture */
        int tex_x = -1; /**< Column in texture. Ignored if tex_id is set to -1*/
        double depth = 0; /**< Depth of the object in the frame (z_buf value). Influences the color*/
        Color color = Color::black; /**< Color is used only if tex_id is -1*/
    };


    /**
     * Pixel buffer for a single frame
     */
    class FrameBuffer {
        buf_size_t width_; /**< Width of buffer in pixels */
        buf_size_t height_; /**< Height of buffer in pixels */
        int int_width_; /**< Integer version of buffer width. Internal-use only */
        int int_height_; /**< Integer version of buffer height. Internal-use only */
        vector<uint32_t> buffer_; /** < flat column-major buffer*/

    public:
        FrameBuffer(int32_t width, int32_t height);

        /**
         * @brief Returns width of the buffer
         */
        buf_size_t width() const;

        /**
         * @brief Returns width of the buffer
         */
        buf_size_t height() const;

        /**
         * @return const pointer to the start of raw buffer data (column-major)
         */
        const uint32_t *raw_data() const;

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
        void circle(int x, int y, double radius, const DrawCallParams &params);

        // draws triangle with given vertices
        void triangle(int x0, int y0, int x1, int y1, int x2, int y2, const DrawCallParams &params);
    };
}
