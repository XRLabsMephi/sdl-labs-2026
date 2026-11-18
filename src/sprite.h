#pragma once

enum class SpriteType {
	food,
	grass,
	teleport
};

struct Sprite {
	double x;
	double y;
	int textureId;
	double scale;
	SpriteType type;
	double elevation = 0.0;
};