#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>
#include "playfield.h"
#include "settings.h"
#include <algorithm>
/*
    Функция вычисления 1 луча. 
    Возвращает расстояние до объекта, с которым произошло пересечение.
    -1 если пересечения в пределах 1.000 не произошло
    Реализовано через DDA, то есть двигаем луч не с каким то шагом, а проверяем клетки на его пути.
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
						return {dist, &board};
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
	return {-1, nullptr};
}

// проверяем, что игрок не касается объектов
bool CheckCollision(double x, double y, double angle, double r, Playfield& pf) {
	for (auto& board : pf.boards) {
		double nearestX = std::clamp(x, board.CoordX, board.CoordX + board.LenghtX);
		double nearestY = std::clamp(y, board.CoordY, board.CoordY + board.LenghtY);
		double dx = x - nearestX;
		double dy = y - nearestY;
		if (dx * dx + dy * dy <= r * r) {
			return false;
		}
	}
	return true;
}

int main(int argc, char* argv[]) {
	// ускорение поворота
	int r_scale = 2;
	double playerRadius = 0.2;
	//ускроение ходьбы
	int w_scale = 4;

	//работа с игровым полем
	int k = settings::map_scale;

	Playfield play_field(10, 10, k);

	play_field.set_rectangle(7, 6, 9, 8, 1);

	play_field.print();


	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cout << SDL_GetError() << '\n';
		return 1;
	}

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	if (!SDL_CreateWindowAndRenderer(
		"Raycasting",
		800,
		600,
		0,
		&window,
		&renderer
	)) {
		std::cout << SDL_GetError() << '\n';
		SDL_Quit();
		return 1;
	}

	//стартовая точка & угол поворота камеры
	double PlayerCoordX = 1.5 * k;
	double PlayerCoordY = 1.5 * k;
	double PlayerA = std::numbers::pi / 4;

	//размеры экрана
	const int screenWidth = 800;
	const int screenHeight = 600;

	//кол-во лучей на экран
	const int raysCount = screenWidth;

	//угол обзора
	const double FOV = std::numbers::pi / 2;

	/*
	    расстояние до плоскости экрана, на который проэцируется все обьекты из точки игрока
	*/
	const double projectionDistance =
		(screenWidth / 2.0) / std::tan(FOV / 2.0);

	bool running = true;

	//основной цикл для отрисовки
	while (running) {
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
		}

		const bool* keys = SDL_GetKeyboardState(nullptr);

		//перемещение
		if (keys[SDL_SCANCODE_A]) {
			PlayerA -= 0.02 * settings::rotation_scale;
		}

		if (keys[SDL_SCANCODE_D]) {
			PlayerA += 0.02 * settings::rotation_scale;
		}
		// Предсказываем перемещение игрока вдоль оси X и Y. Если вдоль одной их них мы окажемся около объекта,
		// то отменяем передвижение вдоль этой оси.
		if (keys[SDL_SCANCODE_W]) {
			if (CheckCollision(PlayerCoordX + std::cos(PlayerA) * w_scale * settings::moving_scale, PlayerCoordY,
			                   PlayerA, playerRadius * settings::map_scale, play_field)) {
				PlayerCoordX += std::cos(PlayerA) * w_scale * settings::moving_scale;
			}
			if (CheckCollision(PlayerCoordX, PlayerCoordY + std::sin(PlayerA) * w_scale * settings::moving_scale,
			                   PlayerA, playerRadius * settings::map_scale, play_field)) {
				PlayerCoordY += std::sin(PlayerA) * w_scale * settings::moving_scale;
			}
		}

		if (keys[SDL_SCANCODE_S]) {
			if (CheckCollision(PlayerCoordX - std::cos(PlayerA) * w_scale * settings::moving_scale, PlayerCoordY,
			                   PlayerA, playerRadius * settings::map_scale, play_field)) {
				PlayerCoordX -= std::cos(PlayerA) * w_scale * settings::moving_scale;
			}
			if (CheckCollision(PlayerCoordX, PlayerCoordY - std::sin(PlayerA) * w_scale * settings::moving_scale,
			                   PlayerA, playerRadius * settings::map_scale, play_field)) {
				PlayerCoordY -= std::sin(PlayerA) * w_scale * settings::moving_scale;
			}
		}

		//закрашиваем весь экран в темно-серый
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		//пол кароч мда
		SDL_FRect floorRect{
			0.0f,
			static_cast<float>(screenHeight / 2),
			static_cast<float>(screenWidth),
			static_cast<float>(screenHeight / 2)
		};

		SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
		SDL_RenderFillRect(renderer, &floorRect);

		//для каждого луча при надобности отрисовываем столкновение со стенкой
		for (int i = 0; i < raysCount; i++) {
			double rayAngle =
				PlayerA - FOV / 2 +
				FOV * i / raysCount;

			auto [distance, cr] = RayTracing(PlayerCoordX, PlayerCoordY, rayAngle, play_field);

			if (cr == nullptr) {
				continue;
			}

			//тут фиксится рыбий глаз
			double correctedDistance =
				distance;
			//*std::cos(rayAngle - PlayerA);
			//за что пофиксили - не ясно, ибо просили пофиксить на этапе сильно позже :/
			// double correctedDistance = distance;

			double height =
				k *
				projectionDistance /
				correctedDistance;

			SDL_FRect rect{
				static_cast<float>(i),
				static_cast<float>(screenHeight / 2.0 - height / 2.0),
				1.0f,
				static_cast<float>(height)
			};

			//коэффициент затухания цвета в зависимости от дальности
			double brightness = 1.0 - correctedDistance / (35.0 * settings::map_scale);
			if (brightness < 0.0) brightness = 0.0;
			if (brightness > 1.0) brightness = 1.0;

			//повышаем контрастность
			brightness = std::pow(brightness, 3);

			SDL_SetRenderDrawColor(renderer, cr->cur_col[0] * brightness, cr->cur_col[1] * brightness,
			                       cr->cur_col[2] * brightness, cr->cur_col[3] * brightness);
			SDL_RenderFillRect(renderer, &rect);
		}

		SDL_RenderPresent(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
