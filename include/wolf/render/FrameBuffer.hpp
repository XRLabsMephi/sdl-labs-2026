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

    [[nodiscard]] bool check_x(int x) const; //проверяем, что x не выходит за границу
    [[nodiscard]] bool check_y(int y) const; //проверяем, что y не выходит за границу

public:
    FrameBuffer(size_t width, size_t height);

    [[nodiscard]] size_t get_width() const;
    [[nodiscard]] size_t get_height() const;
    [[nodiscard]] Color* get_pixels() const;

    void set_pixel(int x, int y, Color color); //отдельному пискелю зададим цвет
    void set_color(Color color); //зададим цвет всем пикселям
    void clear(); //очищаем кадр (по факту задаем color="BLACK")
    void draw_vertical_line(int x, int y_start, int y_end, Color color); //отрисовываем вертикаль
    void draw_horizontal_line(int x_start, int x_end, int y, Color color); //отрисовываем горизонталь

    ~FrameBuffer();
};