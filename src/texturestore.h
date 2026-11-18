#pragma once

#include <cstdint>
#include <string>
#include <vector>

/*
	—обственна€ текстура.
	’ранит пиксели в формате ARGB8888 (https://en.wikipedia.org/wiki/RGBA_color_model).
*/
class Texture {
public:
	// успешно ли файл загружен и распакован в pixels_.
	bool loadFromFile(const std::string& path);

	// 0xAARRGGBB; 0 Ч если координата за границами.
	std::uint32_t sample(int x, int y) const;

	int  width()  const { return width_; }
	int  height() const { return height_; }

	// true Ч есть что рисовать (загрузка прошла успешно).
	bool valid()  const { return width_ > 0 && height_ > 0; }

private:
	int width_ = 0;
	int height_ = 0;
	std::vector<std::uint32_t> pixels_;
};