#pragma once

#include <cstdint>
#include <vector>


/*
	Класс цвета, позволяющий удобно взаимодействовать с незакодированным форматом

	По чесному хранить будем в формате ARGB8888

	По нормальному почитать можно тут https://en.wikipedia.org/wiki/RGBA_color_model
	Картинки нормальные, можно даже что-то понять
*/
struct Color {
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	uint8_t a = 255;
};

/*
	Кадр целиком живёт в оперативной памяти.

	Хранит массив пикселей фиксированного размера в формате ARGB8888
	как в SDL.
*/
class FrameBuffer {
public:
	FrameBuffer(int width, int height);

	int width()  const { return width_; }
	int height() const { return height_; }

	// Залить весь кадр одним цветом.
	void clear(Color c);

	// Изменить цвет одного пикселя.
	void setPixel(int x, int y, Color c);

	// Залить прямоугольник (координаты режутся по границам кадра).
	void fillRect(int x, int y, int w, int h, Color c);

	// Данные для SDL чтобы удобно нарисовать все что понатыкали 
	const uint32_t* data()  const { return pixels_.data(); }
	int pitch() const { return width_ * static_cast<int>(sizeof(uint32_t)); }

private:
	int width_;
	int height_;
	std::vector<uint32_t> pixels_;
};