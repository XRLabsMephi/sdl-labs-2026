#include "menu_button.h"

Button::Button(int x, int y, int width, int height, ButtonType type, int textureId, int hoverTextureId) :
	x(x), y(y), width(width), height(height), type(type), textureId(textureId), hoverTextureId(hoverTextureId) {
}

std::pair<int, int> Button::getCoords() const {
	return {x, y};
}

std::pair<int, int> Button::getSize() const {
	return {width, height};
}

int Button::getTextureId(bool hovered) const {
	if (hovered && hoverTextureId != -1) {
		return hoverTextureId;
	}
	return textureId;
}

ButtonType Button::getType() const {
	return type;
}

bool Button::contains(double mouseX, double mouseY) const {
	return mouseX >= x && mouseX < x + width &&
		mouseY >= y && mouseY < y + height;
}

void Button::setTextures(int textureId, int hoverTextureId) {
	this->textureId = textureId;
	this->hoverTextureId = hoverTextureId;
}