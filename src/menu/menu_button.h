#pragma once

#include <utility>

enum class ButtonType {
	MainMenu,
	Play,
	Hub,
	FirstLevel,
	Records,
	Settings,
	Quit,
	ChangeSensitivity,
	ChangeFOV,
	Placeholder
};

class Button {
public:
	Button(int x, int y, int width, int height, ButtonType type, int textureId, int hoverTextureId = -1);

	std::pair<int, int> getCoords() const;
	std::pair<int, int> getSize() const;

	int getTextureId(bool hovered = false) const;
	ButtonType getType() const;

	bool contains(double mouseX, double mouseY) const;
	void setTextures(int textureId, int hoverTextureId);

private:
	int x, y;
	int width, height;
	int textureId;
	int hoverTextureId;
	ButtonType type;
};