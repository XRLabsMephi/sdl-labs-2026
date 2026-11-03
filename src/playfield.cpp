#include "playfield.h"

#include <fstream>
#include <string>
#include <vector>
#include <iostream>

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
			if (1 <= tp && tp <= 3) {
				boards.push_back(Board(i * k, j * k, 0, k, k, k));
				boards.back().wallId = tp;
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

void Playfield::load_map(const std::string& file) {
    std::ifstream f(file);
    if (!f.is_open()) {
        std::cerr << "! Ошибка: не удалось открыть файл карты :< " << file << "\n";
        return;
    }

    // сгружаем файл в вектор
    std::vector<std::string> lines;
    std::string line;
    int max_width = 0;

    while (std::getline(f, line)) {

        // хотим знать самую длинную строку
        if (static_cast<int>(line.length()) > max_width) {
            max_width = static_cast<int>(line.length());
        }
        lines.push_back(line);
    }

    if (lines.empty()) {
        std::cerr << "! Ошибка: файл карты пуст ! </3 \n";
        return;
    }

    // Задаем новые размеры карты на основе прочитанных данных
    w = max_width;
    h = static_cast<int>(lines.size());

    // Выделяем память под карту заново
    field = std::vector<std::vector<int>>(w, std::vector<int>(h, 0));
    boards.clear();
    sprites.clear();
    teleports.clear();
    max_food = 0;

    // Ставим игрока в безопасное место, если в файле нет 'P'
    player_coords = { -1, -1 };
    std::pair<int, int> last_not_wall = { -1, -1 };
    // 4. Разбираем карту
    for (int y = 0; y < h; ++y) {
        const std::string& current_line = lines[y];

        for (int x = 0; x < w; ++x) {
            // Если строка короче, чем max_width, добиваем её пустотой (0)
            char c = (x < static_cast<int>(current_line.length())) ? current_line[x] : ' ';

            // Безопасная проверка символов
            if (c >= '1' && c <= '3') {
                int wall_id = c - '0';
                field[x][y] = wall_id;  // Записываем в логическую карту
                set_rectangle(x + 1, y + 1, x + 1, y + 1, wall_id);  // Записываем в коллизии
            }
            else {
                last_not_wall = { x, y };
                if (c == '4') {
                    max_food++;
                    field[x][y] = 0; // Пустая клетка
                    sprites.push_back({
                        (x + 0.5) * k,
                        (y + 0.5) * k,
                        6,
                        0.4,
                        SpriteType::food
                        });
                }
                else if (c == '5') {
                    field[x][y] = 0;
                    sprites.push_back({
                        (x + 0.5) * k,
                        (y + 0.5) * k,
                        7,
                        0.8,
                        SpriteType::grass
                        });
                }
                else if (c == 'P') {
                    field[x][y] = 0;
                    player_coords = { x, y };
                }
                else if (c == 'E') {
                    field[x][y] = 0;
                    sprites.push_back({
                        (x + 0.5) * k,
                        (y + 0.5) * k,
                        8,
                        0.8,
                        SpriteType::teleport
                        });
                    teleports.push_back(sprites.back());
                }
                else {
                    // Любой другой символ (пробел, точка, мусор) считаем пустым местом
                    field[x][y] = 0;
                }
            }
        }
    }
    if (player_coords == std::pair<int, int>{-1, -1}&& last_not_wall != std::pair<int, int>{-1, -1}) {
        player_coords = last_not_wall;
    }
    else if (player_coords == std::pair<int, int>{-1, -1}) {
        std::cerr << "! Ошибка: карта не содержит ни 1 возможной позиции для нахождения игрока\nТак нельзя жить ! :c\n";
    }
}

std::pair<int, int> Playfield::get_player_coords() {
	return player_coords;
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
int Playfield::getMax_food() {
    return max_food;
}

std::pair<int, int> Playfield::getPlayer_coords() {
    return player_coords;
}