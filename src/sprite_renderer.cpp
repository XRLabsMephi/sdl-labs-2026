#include "sprite_renderer.h"
#include "settings.h"
#include <cmath>
#include <algorithm>

void SpriteRenderer::draw(
	const Sprite& sprite,
	Player& player,
	FrameBuffer& frameBuffer,
	const std::vector<double>& depthBuffer,
	double projectionDistance,
	const Texture& texture
) {
	double dx = sprite.x - player.get_X();
	double dy = sprite.y - player.get_Y();
	double A = player.get_A();
	double depth = dx * std::cos(A) + dy * std::sin(A);
	double side = -dx * std::sin(A) + dy * std::cos(A);

	if (depth <= 0.001) return;

	double screenX = frameBuffer.width() / 2.0
		+ side * projectionDistance / depth;

	double spriteHeight = settings::map_scale
		* sprite.scale * projectionDistance / depth;

	double spriteWidth = spriteHeight
		* static_cast<double>(texture.width()) / texture.height();

	double bottomY = frameBuffer.height() / 2.0
		+ 0.5 * settings::map_scale * projectionDistance / depth;

	double topY = bottomY - spriteHeight;
	double left = screenX - spriteWidth / 2.0;

	int x0 = std::max(0, static_cast<int>(std::floor(left)));
	int x1 = std::min(frameBuffer.width(),
	                  static_cast<int>(std::ceil(left + spriteWidth)));

	int y0 = std::max(0, static_cast<int>(std::floor(topY)));
	int y1 = std::min(frameBuffer.height(),
	                  static_cast<int>(std::ceil(bottomY)));

	for (int x = x0; x < x1; x++) {
		if (depth >= depthBuffer[x]) continue;

		int texX = std::clamp(
			static_cast<int>((x + 0.5 - left) * texture.width() / spriteWidth),
			0,
			texture.width() - 1
		);

		for (int y = y0; y < y1; y++) {
			int texY = std::clamp(
				static_cast<int>((y + 0.5 - topY) * texture.height() / spriteHeight),
				0,
				texture.height() - 1
			);

			std::uint32_t pixel = texture.sample(texX, texY);

			if ((pixel >> 24) < 128) continue;

			Color color{
				static_cast<std::uint8_t>((pixel >> 16) & 255),
				static_cast<std::uint8_t>((pixel >> 8) & 255),
				static_cast<std::uint8_t>(pixel & 255),
				255
			};

			frameBuffer.setPixel(x, y, color);
		}
	}
}
