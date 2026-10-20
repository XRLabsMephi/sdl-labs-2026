#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>
#include <algorithm>
#include "playfield.h"
#include "player.h"
#include "settings.h"
#include "raytracer.h"

int main(int argc, char* argv[]) {

	//базовое игровое поле
	Playfield play_field(30, 30, settings::map_scale);
	play_field.set_rectangle(5, 5, 10, 10, 1);
	play_field.set_rectangle(3, 2, 5, 5, 1);
	play_field.set_rectangle(12, 2, 15, 10, 1);
	play_field.set_rectangle(2, 12, 27, 12, 1);
	play_field.set_rectangle(20, 20, 25, 25, 1);
	play_field.print();


	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cout << SDL_GetError() << '\n';
		return 1;
	}

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;

	if (!SDL_CreateWindowAndRenderer(
		"Raycasting",
		settings::screenWidth,
		settings::screenHeight,
		0,
		&window,
		&renderer
	)) {
		std::cout << SDL_GetError() << '\n';
		SDL_Quit();
		return 1;
	}

	//кол-во лучей на экран
	const int raysCount = settings::screenWidth;

	//угол обзора
	const double FOV = std::numbers::pi / 2;

	/*
	    расстояние до плоскости экрана, на который проэцируется все обьекты из точки игрока
	*/
	const double projectionDistance =
		(settings::screenWidth / 2.0) / std::tan(FOV / 2.0);

	bool running = true;

	Player main_pl(
		1.5 * settings::map_scale, 
		1.5 * settings::map_scale, 
		std::numbers::pi / 4,
		0.2
	);

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
			main_pl.left();
		}

		if (keys[SDL_SCANCODE_D]) {
			main_pl.right();
		}
		if (keys[SDL_SCANCODE_W]) {
			main_pl.forward(play_field);
		}

		if (keys[SDL_SCANCODE_S]) {
			main_pl.back(play_field);
		}

		//закрашиваем весь экран в темно-серый
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		//пол кароч мда
		SDL_FRect floorRect{
			0.0f,
			static_cast<float>(settings::screenHeight / 2),
			static_cast<float>(settings::screenWidth),
			static_cast<float>(settings::screenHeight / 2)
		};

		SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
		SDL_RenderFillRect(renderer, &floorRect);

		//для каждого луча при надобности отрисовываем столкновение со стенкой
		for (int i = 0; i < raysCount; i++) {
			double rayAngle =
				main_pl.get_A() - FOV / 2 +
				FOV * i / raysCount;

			auto [distance, cr] = RayTracing(main_pl.get_X(), main_pl.get_Y(), rayAngle, play_field);

			if (cr == nullptr) {
				continue;
			}

			//тут фиксится рыбий глаз
			double correctedDistance =
				distance
				* std::cos(rayAngle - main_pl.get_A()); // <- фикс рыбьего глаза fish-fish

			double height =
				settings::map_scale *
				projectionDistance /
				correctedDistance;

			SDL_FRect rect{
				static_cast<float>(i),
				static_cast<float>(settings::screenHeight / 2.0 - height / 2.0),
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
