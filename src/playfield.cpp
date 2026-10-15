#include "playfield.h"

/*
	файл с реализацией функций из playfield.h
*/


Playfield::Playfield(int _w, int _h, int _k) : w(_w), h(_h), k(_k), field(w, std::vector<int>(h, 0)) {
	for (int i = 0; i < w; i++) {
		field[i][0] = 1;
		field[i][h - 1] = 1;
		boards.push_back(Board(i * k, 0, 0, k, k, k));
		if ((i) % 2 == 0) {
			boards.back().cur_col = { 0, 0, 255, 255 };
		}
		else {
			boards.back().cur_col = { 255, 0, 0, 255 };
		}
		boards.push_back(Board(i * k, (h - 1) * k, 0, k, k, k));
		if ((int(i) + int((h - 1))) % 2 == 0) {
			boards.back().cur_col = { 0, 0, 255, 255 };
		}
		else {
			boards.back().cur_col = { 255, 0, 0, 255 };
		}
	}
	for (int i = 0; i < h; i++) {
		field[0][i] = 1;
		field[w - 1][i] = 1;
		boards.push_back(Board(0, i * k, 0, k, k, k));
		if ((int(0) + int(i)) % 2 == 0) {
			boards.back().cur_col = { 0, 0, 255, 255 };
		}
		else {
			boards.back().cur_col = { 255, 0, 0, 255 };
		}
		boards.push_back(Board((w - 1) * k, i * k, 0, k, k, k));
		if ((int((w - 1)) + int(i)) % 2 == 0) {
			boards.back().cur_col = { 0, 0, 255, 255 };
		}
		else {
			boards.back().cur_col = { 255, 0, 0, 255 };
		}
	}
}

void Playfield::set_rectangle(int _x1, int _y1, int _x2, int _y2, int tp) {
	if (_x1 > _x2) std::swap(_x1, _x2);
	if (_y1 > _y2) std::swap(_y1, _y2);
	_x1--;
	_y1--;
	for (int i = _x1; i < _x2; i++) {
		for (int j = _y1; j < _y2; j++) {
			field[i][j] = tp;
			if (tp == 1) {
				boards.push_back(Board(i * k, j * k, 0, k, k, k));
				if ((int(i) + int(j)) % 2 == 0) {
					boards.back().cur_col = { 0, 0, 255, 255 };
				}
				else {
					boards.back().cur_col = { 255, 0, 0, 255 };
				}
			}
		}
	}
}

void Playfield::print() {
	for (int j = w - 1; j >= 0; j--) {
		for (int i = 0; i < h; i++) {
			std::cout << field[j][i];
		}
		std::cout << '\n';
	}
}

std::vector<std::vector<int>> Playfield::getField() {
	return field;
}

int Playfield::getHight() {
	return h;
}
int Playfield::getWidth() {
	return w;
}