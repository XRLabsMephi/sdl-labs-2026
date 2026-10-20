#include <cmath>
#include "playfield.h"
#include "settings.h"
/*
	файл с реализацией функций из raytracer.h
*/


std::pair<double, Board*> RayTracing(double startPosX, double startPosY, double angle, Playfield& pf) {
	std::vector<std::vector<int>> field = pf.getField();
	// координаты в таблице карты
	std::pair<int, int> CoordsInFireld = {
		int(std::floor(startPosX / settings::map_scale)),
		int(std::floor(startPosY / settings::map_scale))
	};
	// куда смотрим в проекциях на XY
	int stepX = std::cos(angle) >= 0 ? 1 : -1;
	int stepY = std::sin(angle) >= 0 ? 1 : -1;

	double dist = 0;

	while (dist < 30 * settings::map_scale) {
		if (CoordsInFireld.first >= 0 && CoordsInFireld.second >= 0 &&
			CoordsInFireld.first < int(field.size()) &&
			CoordsInFireld.second < int(field[CoordsInFireld.first].size())) {
			// Если в клетке стоит объект, не воздух, возвращаем результат
			if (field[CoordsInFireld.first][CoordsInFireld.second] != 0) {
				for (auto& board : pf.boards) {
					if (board.CoordX / settings::map_scale == CoordsInFireld.first &&
						board.CoordY / settings::map_scale == CoordsInFireld.second) {
						return { dist, &board };
					}
				}
			}
		}

		double cornerX = (CoordsInFireld.first + (stepX == 1)) * settings::map_scale;
		double cornerY = (CoordsInFireld.second + (stepY == 1)) * settings::map_scale;
		// определяем в какую клетку следующей переместиться, отдельно рассмотрим углы в 90 градусов
		if (std::abs(std::cos(angle)) < 1e-10) {
			dist = (cornerY - startPosY) / std::sin(angle);
			CoordsInFireld.second += stepY;
		}
		else if (std::abs(std::sin(angle)) < 1e-10) {
			dist = (cornerX - startPosX) / std::cos(angle);
			CoordsInFireld.first += stepX;
		}
		else {
			double y = startPosY + (cornerX - startPosX) * std::tan(angle);

			if ((y - cornerY) * stepY >= 0) {
				dist = (cornerY - startPosY) / std::sin(angle);
				CoordsInFireld.second += stepY;
			}
			else {
				dist = (cornerX - startPosX) / std::cos(angle);
				CoordsInFireld.first += stepX;
			}
		}
	}
	return { -1, nullptr };
}