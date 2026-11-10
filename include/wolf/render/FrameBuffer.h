#pragma once

#include <cstdint>
#include <vector>

#include "Fog.h"
#include "TextureStorage.h"
#include "core/Color.h"

namespace render {
    using Color = core::Color;
    template<typename T>
    using vector = std::vector<T>;
    using buf_size_t = size_t;

    /**
    * @brief Contains parameters for texture draw call to @c FrameBuffer.
    */
    struct DrawCallParams {
        const Uint32* tex_col = nullptr; /**< Texture pixels column*/
        int tex_y_start = 0; /**< Mapping from screen to texture coordinates in 16.16 fixed precision format.*/
        int tex_step = 0; /**< Texture pixels step per screen pixel, in 16.16 fixed precision format*/
        double depth = 0; /**< Depth of the object in the frame (z_buf value). Influences the color*/
        Uint8 brightness = 0; /**< Dynamic shadowing coefficient*/
    };

    /**
     * Pixel buffer for a single frame
     */
    class FrameBuffer {
        buf_size_t width_; /**< Width of buffer in pixels */
        buf_size_t height_; /**< Height of buffer in pixels */
        int int_width_; /**< Integer version of buffer width. Internal-use only */
        int int_height_; /**< Integer version of buffer height. Internal-use only */
        vector<Uint32> buffer_; /**< flat column-major buffer*/
        Fog fog_;

    public:
        FrameBuffer(int32_t width, int32_t height);

        /**
         * @brief Sets the fog parameters: color and factor
         */
        void set_fog_params(Color fog_color, float min_fog_depth, float max_fog_depth);

        /**
         * @brief Sets the fog parameters: color and factor
         */
        /**
         * @brief Returns width of the buffer
         */
        [[nodiscard]] buf_size_t width() const;

        /**
         * @brief Returns width of the buffer
         */
        [[nodiscard]] buf_size_t height() const;

        /**
         * @return const pointer to the start of raw buffer data (column-major)
         */
        [[nodiscard]] const uint32_t *raw_data() const;

        /**
         * @brief Fills whole buffer
         * @param color the color to apply
         */
        void clear(Color color);

        /**
         * @brief Colors single pixel at point
         * @param x x coordinate
         * @param y y coordinate
         * @param color color to apply
         * @attention Points outside buffer rejects
         */
        void set(int x, int y, Color color);

        /**
        * @brief Draws vertical line
        * @param x x coordinate
        * @param y0 start y coordinate
        * @param y1 end y coordinate
        * @param color color to apply
        */
        void vline(int x, int y0, int y1, Color color);

        /**
         * @brief Draws vertical line
         * @param x x coordinate
         * @param y0 start y coordinate
         * @param y1 end y coordinate
         * @param params draw call parameters (see @c DrawCallParams)
         */
        void vline(int x, int y0, int y1, const DrawCallParams &params);

        /**
         * @brief Draws vertical line
         * @param x x coordinate
         * @param y0 start y coordinate
         * @param y1 end y coordinate
         * @param params draw call parameters (see @c DrawCallParams)
         */
        void vline_sprite(int x, int y0, int y1, const DrawCallParams &params);

        /**
         * @brief Draws horizontal line
         * @param y y coordinate
         * @param x0 start x coordinate
         * @param x1 end x coordinate
         * @param color color to apply
         */
        void hline(int y, int x0, int x1, Color color);

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
         * @param color color to apply
         */
        void rectangle(int x0, int y0, int x1, int y1, Color color);

        /**
         * @brief Draws rectangle.
         * @param x0 top-left corner x coordinate
         * @param y0 top-left y coordinate
         * @param x1 bottom-right x coordinate
         * @param y1 bottom-right y coordinate
         * @param params draw call parameters (see @c DrawCallParams)
         */
        void rectangle(int x0, int y0, int x1, int y1, DrawCallParams &params);
    };
}
