#include <cmath>
#include "raytracer.h"
#include "playfield.h"
#include "settings.h"
/*
	файл с реализацией функций из raytracer.h
*/


RayHit RayTracing(double startPosX, double startPosY, double angle, double playerDir, Playfield& pf) {
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

	// Через какую грань мы вошли в текущую клетку.
	// На первой итерации (dist = 0) значения не имеет.
	int lastSide = 0;

	while (dist < 30 * settings::map_scale) {
		if (CoordsInFireld.first >= 0 && CoordsInFireld.second >= 0 &&
			CoordsInFireld.first < int(field.size()) &&
			CoordsInFireld.second < int(field[CoordsInFireld.first].size())) {
			// Если в клетке стоит объект, не воздух, возвращаем результат
			if (field[CoordsInFireld.first][CoordsInFireld.second] != 0) {

				for (auto& board : pf.boards) {
					if (board.CoordX / settings::map_scale == CoordsInFireld.first &&
						board.CoordY / settings::map_scale == CoordsInFireld.second) {

						RayHit hit;
						hit.hit = true;
						hit.len = dist;
						//hit.depth = dist;
						hit.depth = dist * std::cos(angle - playerDir);// - ФИКС РЫБЬЕГО ГЛАЗА
						hit.mapX = CoordsInFireld.first;
						hit.mapY = CoordsInFireld.second;
						hit.wallId = board.wallId;
						hit.side = lastSide;


						// Точка попадания
						const double hitX = startPosX + dist * std::cos(angle);
						const double hitY = startPosY + dist * std::sin(angle);

						// вдоль вертикальной грани меняется Y, вдоль горизонтальной — X
						const double coord = (lastSide == 0) ? hitY : hitX;
						double u = coord / settings::map_scale;
						u -= std::floor(u); //[0, 1)
						hit.wallU = u;

						// зеркалим, чтобы текстура не была перевёрнутой
						//if (lastSide == 0 && std::cos(angle) > 0) hit.wallU = 1.0 - hit.wallU;
						//if (lastSide == 1 && std::sin(angle) < 0) hit.wallU = 1.0 - hit.wallU;

						return hit;
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
			lastSide = 1;
		}
		else if (std::abs(std::sin(angle)) < 1e-10) {
			dist = (cornerX - startPosX) / std::cos(angle);
			CoordsInFireld.first += stepX;
			lastSide = 0;
		}
		else {
			double y = startPosY + (cornerX - startPosX) * std::tan(angle);

			if ((y - cornerY) * stepY >= 0) {
				dist = (cornerY - startPosY) / std::sin(angle);
				CoordsInFireld.second += stepY;
				lastSide = 1;
			}
			else {
				dist = (cornerX - startPosX) / std::cos(angle);
				CoordsInFireld.first += stepX;
				lastSide = 0;
			}
		}
	}
	return RayHit{};
}