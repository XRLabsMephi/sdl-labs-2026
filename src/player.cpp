#include <cmath>
#include <numbers>
#include <vector>
#include <algorithm>
#include "playfield.h"
#include "player.h"
#include "settings.h"

/*
	файл c реализацией методов из player.h
*/

Player::Player(double _x, double _y, double _A, double _r) : x(_x), y(_y), A(_A), r(_r) {};

bool Player::CheckCollision(double _x, double _y, Playfield& pf) {
	for (auto& board : pf.boards) {
		double nearestX = std::clamp(_x, board.CoordX, board.CoordX + board.LenghtX);
		double nearestY = std::clamp(_y, board.CoordY, board.CoordY + board.LenghtY);
		double dx = _x - nearestX;
		double dy = _y - nearestY;
		if (dx * dx + dy * dy <= r * r * settings::map_scale * settings::map_scale) {
			return false;
		}
	}
	return true;
}

void Player::turn_left() {
	A -= 0.02 * settings::rotation_scale;
}

void Player::turn_right() {
	A += 0.02 * settings::rotation_scale;
}

// для forward и back :
// Предсказываем перемещение игрока вдоль оси X и Y. Если вдоль одной их них мы окажемся около объекта,
// то отменяем передвижение вдоль этой оси.


void Player::forward(Playfield& pf) {
	if (this->CheckCollision(x + std::cos(A) * settings::moving_scale, y, pf)) {
		x += std::cos(A) * settings::moving_scale;
	}
	if (this->CheckCollision(x, y + std::sin(A) * settings::moving_scale, pf)) {
		y += std::sin(A) * settings::moving_scale;
	}
}

void Player::back(Playfield& pf) {
	if (this->CheckCollision(x - std::cos(A) * settings::moving_scale, y, pf)) {
		x -= std::cos(A) * settings::moving_scale;
	}
	if (this->CheckCollision(x, y - std::sin(A) * settings::moving_scale, pf)) {
		y -= std::sin(A) * settings::moving_scale;
	}
}

void Player::left(Playfield& pf) {
	if (this->CheckCollision(x + std::cos(A - std::numbers::pi / 2) * settings::moving_scale, y, pf)) {
		x += std::cos(A - std::numbers::pi / 2) * settings::moving_scale;
	}
	if (this->CheckCollision(x, y + std::sin(A - std::numbers::pi / 2) * settings::moving_scale, pf)) {
		y += std::sin(A - std::numbers::pi / 2) * settings::moving_scale;
	}
}

void Player::right(Playfield& pf) {
	if (this->CheckCollision(x + std::cos(A + std::numbers::pi / 2) * settings::moving_scale, y, pf)) {
		x += std::cos(A + std::numbers::pi / 2) * settings::moving_scale;
	}
	if (this->CheckCollision(x, y + std::sin(A + std::numbers::pi / 2) * settings::moving_scale, pf)) {
		y += std::sin(A + std::numbers::pi / 2) * settings::moving_scale;
	}
}

double Player::get_A() { return A; }
double Player::get_X() { return x; }
double Player::get_Y() { return y; }