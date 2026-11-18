#include "menu_renderer.h"

#include <algorithm>
#include <cstdint>
#include <SDL3/SDL.h>
#include <cmath>
#include <numbers>
#include "../settings.h"

void MenuRenderer::drawTexture(
	FrameBuffer& frameBuffer,
	const Texture& texture,
	int x,
	int y,
	int width,
	int height
) const {
	if (!texture.valid() || width <= 0 || height <= 0) {
		return;
	}

	int x0 = std::max(0, x);
	int y0 = std::max(0, y);
	int x1 = std::min(frameBuffer.width(), x + width);
	int y1 = std::min(frameBuffer.height(), y + height);

	for (int py = y0; py < y1; py++) {
		int texY = std::clamp(
			(py - y) * texture.height() / height,
			0,
			texture.height() - 1
		);

		for (int px = x0; px < x1; px++) {
			int texX = std::clamp(
				(px - x) * texture.width() / width,
				0,
				texture.width() - 1
			);

			std::uint32_t pixel = texture.sample(texX, texY);

			if ((pixel >> 24) < 128) {
				continue;
			}

			Color color{
				static_cast<std::uint8_t>((pixel >> 16) & 255),
				static_cast<std::uint8_t>((pixel >> 8) & 255),
				static_cast<std::uint8_t>(pixel & 255),
				255
			};

			frameBuffer.setPixel(px, py, color);
		}
	}
}

void MenuRenderer::render(const MenuScreen& screen, FrameBuffer& frameBuffer, std::span<const Texture> textures,
                          int hoveredButton) const {
	frameBuffer.clear(Color{20, 25, 35, 255});

	auto findTexture = [&](int id) -> const Texture* {
		if (id < 0 || static_cast<std::size_t>(id) >= textures.size()) {
			return nullptr;
		}

		const Texture& texture = textures[id];
		return texture.valid() ? &texture : nullptr;
	};

	if (const Texture* background = findTexture(screen.backgroundTextureId)) {
		drawScrollingBackground(frameBuffer, *background);
	}

	for (int i = 0; i < static_cast<int>(screen.buttons.size()); i++) {
		const Button& button = screen.buttons[i];

		auto [x, y] = button.getCoords();
		auto [width, height] = button.getSize();
		x += (frameBuffer.width() - settings::screenWidth) / 2;
		y += (frameBuffer.height() - settings::screenHeight) / 2;

		bool hovered = (i == hoveredButton);
		int textureId = button.getTextureId(hovered);

		if (const Texture* texture = findTexture(textureId)) {
			drawTexture(frameBuffer, *texture, x, y, width, height);
		}
		else {
			frameBuffer.fillRect(
				x,
				y,
				width,
				height,
				hovered
					? Color{110, 130, 165, 255}
					: Color{85, 85, 85, 255}
			);
		}
	}
}

void MenuRenderer::drawScrollingBackground(FrameBuffer& frameBuffer, const Texture& texture) const {
	if (!texture.valid()) return;

	int screenWidth = frameBuffer.width();
	int screenHeight = frameBuffer.height();
	if (screenWidth <= 0 || screenHeight <= 0) return;

	int imageWidth = std::max(screenWidth,
	                          static_cast<int>(std::round(
		                          static_cast<double>(texture.width()) * screenHeight / texture.height())));
	int maxOffset = imageWidth - screenWidth;

	double time = SDL_GetTicks() / 1000.0;
	double progress = (1.0 - std::cos(2.0 * std::numbers::pi * time / 120.0)) / 2.0;
	int offset = static_cast<int>(maxOffset * progress);

	drawTexture(frameBuffer, texture, -offset, 0, imageWidth, screenHeight);
}
