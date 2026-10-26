#include "framebuffer.h"

#include <algorithm>

/*
	реализация функций из framebuffer.h
*/

// Color в формат 0xAARRGGBB.
static uint32_t pack(Color c) {
	return (static_cast<uint32_t>(c.a) << 24) |
		(static_cast<uint32_t>(c.r) << 16) |
		(static_cast<uint32_t>(c.g) << 8) |
		static_cast<uint32_t>(c.b);
}

FrameBuffer::FrameBuffer(int width, int height)
	: width_(width),
	height_(height),
	pixels_(static_cast<size_t>(width)* static_cast<size_t>(height),
		pack(Color{ 0, 0, 0, 255 })) {}

void FrameBuffer::clear(Color c) {
	std::fill(pixels_.begin(), pixels_.end(), pack(c));
}

void FrameBuffer::setPixel(int x, int y, Color c) {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return;
	pixels_[static_cast<size_t>(y) * width_ + x] = pack(c);
}

void FrameBuffer::fillRect(int x, int y, int w, int h, Color c) {
	int x0 = std::max(0, x);
	int y0 = std::max(0, y);
	int x1 = std::min(width_, x + w);
	int y1 = std::min(height_, y + h);
	if (x0 >= x1 || y0 >= y1) return;

	uint32_t cr = pack(c);
	for (int r = y0; r < y1; r++) {
		auto first = pixels_.begin() + static_cast<size_t>(r) * width_ + x0;
		std::fill(first, first + (x1 - x0), cr);
	}
}