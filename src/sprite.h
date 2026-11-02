#pragma once

enum class SpriteType {
	Steak,
	Barrel
};

struct Sprite {
	double x;
	double y;
	int textureId;
	double scale;
	SpriteType type;
};