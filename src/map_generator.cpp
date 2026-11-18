#include "map_generator.h"

#include <vector>
#include <random>
#include <algorithm>
#include <deque>
#include <string> 
#include <fstream>

using namespace std;

vector<vector<int>> generate_map(int w, int h, int max_food, int min_food_delt, int tp_count) {
    //базовое заполнение
    vector<vector<int>> map(w, vector<int>(h, '1'));

    //а бортики ?
    for (int x = 0; x < w; ++x) {
        map[x][0] = '2';
        map[x][h - 1] = '2';
    }
    for (int y = 0; y < h; ++y) {
        map[0][y] = '2';
        map[w - 1][y] = '2';
    }

    //генератор случайных чисел
    random_device rd;
    mt19937 gen(rd());

    // Выбор стартовой точки (игрок = 'P') строго внутри поля
    int startX = uniform_int_distribution<int>(2, w - 3)(gen);
    int startY = uniform_int_distribution<int>(2, h - 3)(gen);
    map[startX][startY] = 'P';

    // очердь новых клеток
    vector<pair<int, int>> carved;
    carved.push_back({ startX, startY });

    // Целевое количество выкопанных клеток ( пока пусть 75% )
    int target_carved = static_cast<int>((w - 2) * (h - 2) * 0.4);

    int currX = startX;
    int currY = startY;

    // Направления: вверх, вправо, вниз, влево
    int dx[] = { 0, 1, 0, -1 };
    int dy[] = { -1, 0, 1, 0 };

    int attempts = 0;
    const int MAX_ATTEMPTS = 10000;

    // Random BFS    
    while (static_cast<int>(carved.size()) < target_carved && attempts < MAX_ATTEMPTS) {
        attempts++;
        // С шансом 50% продолжаем копать с текущего места, иначе прыгаем в случайную выкопанную точку
        if (uniform_int_distribution<int>(0, 10)(gen) == 0) {
            auto p = carved[uniform_int_distribution<size_t>(0, carved.size() - 1)(gen)];
            currX = p.first;
            currY = p.second;
        }

        int dir = uniform_int_distribution<int>(0, 3)(gen);
        int nx = currX + dx[dir];
        int ny = currY + dy[dir];

        // Проверяем, что не выходим за границы (оставляем рамку из '2' целой)
        if (nx > 1 && nx < w - 2 && ny > 1 && ny < h - 2) {
            if (map[nx][ny] == '1') { // Если это стена
                map[nx][ny] = '0';    // Выкапываем пустую клетку
                carved.push_back({ nx, ny });
                currX = nx;
                currY = ny;
            }
        }
    }

    // Собираем все пустые клетки для расстановки объектов
    vector<pair<int, int>> empty_tiles;
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            if (map[x][y] == '0') {
                empty_tiles.push_back({ x, y });
            }
        }
    }

    // Перемешиваем пустые клетки, чтобы объекты ложились случайно
    shuffle(empty_tiles.begin(), empty_tiles.end(), gen);

    // Расстановка телепортов ('E')
    int placed_tp = 0;
    for (auto& p : empty_tiles) {
        if (placed_tp >= tp_count) break;
        map[p.first][p.second] = 'E';
        placed_tp++;
    }

    // Расстановка еды ('4')
    int placed_food = 0;
    for (auto& p : empty_tiles) {
        if (map[p.first][p.second] != '0') continue; // Пропускаем, если уже занято
        if (placed_food >= max_food) break;
        map[p.first][p.second] = '4';
        placed_food++;
    }

    // Расстановка кустов ('5')
    for (auto& p : empty_tiles) {
        if (map[p.first][p.second] != '0') continue; // Пропускаем занятые
        if (uniform_int_distribution<int>(0, 99)(gen) < 10) { // 20% шанс на куст
            map[p.first][p.second] = '5';
        }
    }

    // 8. Правило 5: блоки, соседствующие с едой (в т.ч. по углам), становятся бревном ('3')
    int ndx[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
    int ndy[] = { -1, 0, 1, -1, 1, -1, 0, 1 };

    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            if (map[x][y] == '4') { // Если это еда
                for (int i = 0; i < 8; ++i) {
                    int nx = x + ndx[i];
                    int ny = y + ndy[i];
                    if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                        // Если рядом обычная стена, меняем её на обтесанное бревно
                        if (map[nx][ny] == '1') {
                            map[nx][ny] = '3';
                        }
                    }
                }
            }
        }
    }

    return map;
}

void save_map_to_file(const string& filename, const vector<vector<int>>& map) {
    ofstream f(filename);
    if (!f.is_open()) return;

    // Внимание: внешний цикл по Y (высота), внутренний по X (ширина)
    for (int y = 0; y < map[0].size(); ++y) {
        for (int x = 0; x < map.size(); ++x) {
            f << static_cast<char>(map[x][y]);
        }
        f << "\n";
    }
}