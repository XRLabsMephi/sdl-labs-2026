#include "application.h"

#include <SDL3/SDL.h>
#include <iostream>
#include <cmath>
#include <numbers>
#include <algorithm>

#include "framebuffer.h"
#include "sdl_presenter.h"
#include "playfield.h"
#include "player.h"
#include "settings.h"
#include "raytracer.h"

Application::Application() = default;

Application::~Application() {
	if (renderer) { SDL_DestroyRenderer(renderer); renderer = nullptr; }
	if (window) { SDL_DestroyWindow(window);     window = nullptr; }
	SDL_Quit();  // безопасно и повторно
}


static Color toColor(std::uint32_t argb, double brightness) {
	const auto r = static_cast<std::uint8_t>(((argb >> 16) & 255) * brightness);
	const auto g = static_cast<std::uint8_t>(((argb >> 8) & 255) * brightness);
	const auto b = static_cast<std::uint8_t>((argb & 255) * brightness);
	return Color{ r, g, b, 255 };
}

int Application::run() {
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

	if (!SDL_CreateWindowAndRenderer(
		"Genuine Minecraft",
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

	//загружаем текстуры (пока мало - ручками)
	textures_[1].loadFromFile("textures/wall01.bmp");
	textures_[2].loadFromFile("textures/floor.bmp");
	textures_[3].loadFromFile("textures/skybox.bmp");

	/* 
		Область видимости нужна, чтобы frameBuffer и presenter
		разрушились раньше, чем renderer/window/SDL_Quit
	*/ 
	{
		FrameBuffer  frameBuffer(settings::screenWidth, settings::screenHeight);
		SdlPresenter presenter(renderer, settings::screenWidth, settings::screenHeight);

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
				main_pl.left(play_field);
				if (keys[SDL_SCANCODE_LSHIFT]) main_pl.left(play_field);
			}
			if (keys[SDL_SCANCODE_D]) {
				main_pl.right(play_field);
				if (keys[SDL_SCANCODE_LSHIFT]) main_pl.right(play_field);
			}
			if (keys[SDL_SCANCODE_LEFT]) {
				main_pl.turn_left();
			}
			if (keys[SDL_SCANCODE_RIGHT]) {
				main_pl.turn_right();
			}
			if (keys[SDL_SCANCODE_W]) {
				main_pl.forward(play_field);
				if (keys[SDL_SCANCODE_LSHIFT]) main_pl.forward(play_field);
			}
			if (keys[SDL_SCANCODE_S]) {
				main_pl.back(play_field);
				if (keys[SDL_SCANCODE_LSHIFT]) main_pl.back(play_field);
			}



			//хотим красивый скайбокс
			const Texture& skyTex = textures_[3];
			if (skyTex.valid()) {
				// Мы рисуем небо только в верхней половине экрана (над горизонтом)
				const int horizon = settings::screenHeight / 2;

				for (int x = 0; x < settings::screenWidth; ++x) {
					//Вычисляем угол луча для текущего столбца
					double cameraX = 2.0 * x / settings::screenWidth - 1.0;
					double rayAngle = main_pl.get_A() + std::atan(cameraX * std::tan(FOV / 2.0));

					//Переводим угол в координату X на текстуре
					//Ограничиваем угол от 0 до 2*PI (полный круг)
					double normAngle = std::fmod(rayAngle, 2.0 * std::numbers::pi);
					if (normAngle < 0) normAngle += 2.0 * std::numbers::pi;

					// Вычисляем столбец текстуры
					int texX = static_cast<int>((normAngle / (2.0 * std::numbers::pi)) * skyTex.width());
					if (texX < 0) texX = 0;
					if (texX >= skyTex.width()) texX = skyTex.width() - 1;

					//Рисуем столбец неба сверху вниз
					for (int y = 0; y < horizon; ++y) {
						// Растягиваем текстуру по высоте
						int texY = static_cast<int>((static_cast<double>(y) / horizon) * skyTex.height());
						if (texY < 0) texY = 0;
						if (texY >= skyTex.height()) texY = skyTex.height() - 1;
						frameBuffer.setPixel(x, y, toColor(skyTex.sample(texX, texY), 1.0));
					}
				}
			}

			// Хотим кароч карсивый пол
			const Texture& floorTex = textures_[2];
			const double FOV_half = FOV / 2.0;
			const double leftAngle = main_pl.get_A() - FOV_half;
			const double rightAngle = main_pl.get_A() + FOV_half;

			// Направления крайних лучей экрана
			const double rayDirX0 = std::cos(leftAngle);
			const double rayDirY0 = std::sin(leftAngle);
			const double rayDirX1 = std::cos(rightAngle);
			const double rayDirY1 = std::sin(rightAngle);

			if (floorTex.valid()) {
				// Идем по строкам: от горизонта вниз (пол) и от горизонта вверх (потолок)
				for (int y = (settings::screenHeight / 2) + 1e-9; y < settings::screenHeight; ++y) {
					// Расстояние от горизонта
					//if (settings::screenHeight / 2 - y == 0) continue; // Сама линия горизонта, чтобы не делить на 0

					// Расстояние до пола/потолка для этой строки
					double rowDistance = (0.5 * settings::map_scale * projectionDistance) / abs(settings::screenHeight / 2 - y);

					double rayDist = rowDistance / std::cos(FOV / 2.0);

					// Шаг текстуры вдоль строки
					double stepX = rayDist * (rayDirX1 - rayDirX0) / settings::screenWidth;
					double stepY = rayDist * (rayDirY1 - rayDirY0) / settings::screenWidth;

					// Начальные координаты в мире для левого края экрана
					double floorX = main_pl.get_X() + rayDist * rayDirX0;
					double floorY = main_pl.get_Y() + rayDist * rayDirY0;

					//затухание в соответствии с расстоянием 
					double brightness = 1.0 - rayDist / (35.0 * settings::map_scale);
					if (brightness < 0.0) brightness = 0.0;
					if (brightness > 1.0) brightness = 1.0;

					//повышаем контрастность
					brightness = std::pow(brightness, 3);

					for (int x = 0; x < settings::screenWidth; x++) {
						// Получаем координаты внутри клетки (0.0 - 1.0)
						double texU = floorX / settings::map_scale;
						texU -= std::floor(texU);
						double texV = floorY / settings::map_scale;
						texV -= std::floor(texV);

						int texX = static_cast<int>(texU * floorTex.width());
						int texY = static_cast<int>(texV * floorTex.height());

						// Защита от выхода за границы
						if (texX < 0) texX = 0;
						if (texX >= floorTex.width()) texX = floorTex.width() - 1;
						if (texY < 0) texY = 0;
						if (texY >= floorTex.height()) texY = floorTex.height() - 1;

						frameBuffer.setPixel(x, y, toColor(floorTex.sample(texX, texY), brightness));

						// Шагаем к следующему пикселю строки
						floorX += stepX;
						floorY += stepY;
					}
				}
			}
			else {
				// Fallback, если текстура пола не загрузилась
				frameBuffer.fillRect(0, settings::screenHeight / 2, settings::screenWidth, settings::screenHeight / 2, Color{ 255, 0, 0, 255 });
			}

			//для каждого луча при надобности отрисовываем столкновение со стенкой
			for (int i = 0; i < raysCount; i++) {
				
				double cameraX = 2.0 * i / raysCount - 1.0; // [-1:1] - коэф от левого до правого края.

				// Истинный угол луча с учетом плоской проекции
				double rayAngle = main_pl.get_A() + std::atan(cameraX * std::tan(FOV / 2.0));

				//double rayAngle = main_pl.get_A() - FOV / 2 + FOV * i / raysCount;


				RayHit hit = RayTracing(main_pl.get_X(), main_pl.get_Y(), rayAngle, main_pl.get_A() ,play_field);
				if (!hit.hit) continue;


				int height = static_cast<int>(settings::map_scale * projectionDistance / hit.depth);
				int yStart = static_cast<int>(settings::screenHeight / 2.0 - height / 2.0);

				//затухание в соответствии с расстоянием 
				double brightness = 1.0 - hit.depth / (35.0 * settings::map_scale);
				if (brightness < 0.0) brightness = 0.0;
				if (brightness > 1.0) brightness = 1.0;

				//повышаем контрастность
				brightness = std::pow(brightness, 3);

				const Texture& tex = textures_[hit.wallId];

				if (!tex.valid()) {

					Color c { 255, 0, 0, 255};
					frameBuffer.fillRect(i, yStart, 1, height, c);
					//depthBuffer_[i] = hit.depth;
					continue;
				}


				

				int texX = static_cast<int>(hit.wallU * tex.width());
				if (texX >= tex.width()) texX = tex.width() - 1;

				const double texStep = static_cast<double>(tex.height()) / height;
				const int y0 = std::max(0, yStart);
				const int y1 = std::min(static_cast<int>(settings::screenHeight), yStart + height);
				double texPos = (y0 - yStart) * texStep;

				for (int y = y0; y < y1; ++y) {
					int texY = static_cast<int>(texPos) % tex.height();
					texPos += texStep;

					frameBuffer.setPixel(i, y, toColor(tex.sample(texX, texY), brightness));
				}
				//depthBuffer_[i] = hit.depth;
			}

			//готовый кадр отдаём презентеру
			presenter.present(frameBuffer);
		}
	}

	SDL_DestroyRenderer(renderer);
	renderer = nullptr;
	SDL_DestroyWindow(window);
	window = nullptr;
	SDL_Quit();

	return 0;
}