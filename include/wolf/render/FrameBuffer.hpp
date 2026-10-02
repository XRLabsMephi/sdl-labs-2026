#pragma once
#include "core/Color.hpp"
#include <cstddef>

/*
FrameBuffer - структура данных, которая содержит информацию о пикселях.
Это блок (двумерный массив), в каждой ячейке которого хранится информация о пикселе - его цвет.
Блок размера width x height.
*/

class FrameBuffer {
    size_t width_; //ширина блока
    size_t height_; //высота блока
    Color* pixels_; //массив пикселей

    bool check_x(size_t x) const; //проверяем, что x не выходит за границу

    bool check_y(size_t y) const; //проверяем, что y не выходит за границу

public:
    FrameBuffer(size_t width, size_t height);

    inline size_t get_width() const;

    inline size_t get_height() const;

    inline Color* get_pixels() const;

    void set_pixel(size_t x, size_t y, Color color); //отдельному пискелю зададим цвет

    void set_color(Color color); //зададим цвет всем пикселям

    void clear(); //очищаем кадр (по факту задаем color="BLACK")

    void draw_vertical_line(size_t x, size_t y_start, size_t y_end, color); //отрисовываем вертикаль

    void draw_horizontal_line(size_t x_start, size_t x_end, size_t y, color); //отрисовываем горизонталь

    ~FrameBuffer();
};