#include <cmath>
#include <numbers>
#include <vector>
#include "playfield.h"
#include "player.h"
#include "settings.cpp"
/*
	файл c реализацией методов из player.h
*/

Player::Player(double _x, double _y, double _A, double _r) : x(_x), y(_y), A(_A), r(_r) {};

vector<pair<double, double>> Player::points() {
	return { {x - r, y}, {x, y - r}, {x + r, y}, {x, y + r} };
}

void Player::left() {
	A -= 0.02 * settings::rotation_scale;
}

void Player::right() {
	A += 0.02 * settings::rotation_scale;
}

void Player::forward(Playfield& p_f) {
	auto& borders = p_f.boards;
	
	bool forw_point = true;
	bool left_point = true;
	bool right_point = true;

	for (auto& el : borders) {
		
	}
}
