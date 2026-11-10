#pragma once

#include <span>
#include "menu_screen.h"
#include "../framebuffer.h"
#include "../texturestore.h"

class MenuRenderer {
public:
	void render(const MenuScreen& screen, FrameBuffer& frameBuffer, std::span<const Texture> textures,
	            int hoveredButton = -1) const;

private:
	void drawTexture(FrameBuffer& frameBuffer, const Texture& texture, int x, int y, int width, int height) const;
	void drawScrollingBackground(FrameBuffer& frameBuffer, const Texture& texture) const;
};
