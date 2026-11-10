#include "texturestore.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstddef>
#include <cstring>

/*
	Файл с реализацией из texturestore.h
*/

bool Texture::loadFromFile(const std::string& path) {
	SDL_Surface* raw = SDL_LoadBMP(path.c_str());
	if (raw == nullptr) {
		SDL_Log("Texture: cannot load %s: %s", path.c_str(), SDL_GetError());
		return false;
	}

	// приводим к тому же формату, что FrameBuffer
	SDL_Surface* conv = SDL_ConvertSurface(raw, SDL_PIXELFORMAT_ARGB8888);
	SDL_DestroySurface(raw);
	if (conv == nullptr) {
		SDL_Log("Texture: convert failed: %s", SDL_GetError());
		return false;
	}
	
	width_ = conv->w;
	height_ = conv->h;
	pixels_.assign(static_cast<std::size_t>(width_) * height_, 0);

	// построчно, с учётом pitch (у поверхности может быть выравнивание)
	const auto* src = static_cast<const std::uint8_t*>(conv->pixels);
	for (int y = 0; y < height_; ++y) {
		std::memcpy(pixels_.data() + y * width_, src + y * conv->pitch, width_ * sizeof(std::uint32_t));
	}

	SDL_DestroySurface(conv);
	return true;
}

std::uint32_t Texture::sample(int x, int y) const {
	if (x < 0 || y < 0 || x >= width_ || y >= height_) return 0;
	return pixels_[static_cast<std::size_t>(y) * width_ + x];
}