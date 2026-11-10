#include "settings.h"  
#include "raytracer.h"
#include "frame_renderer.h"
#include "menu/game_settings.h" 

#include <thread>
#include <future>
#include <iostream>
/*
	Файл с реализацией функций из frame_renderer.h
*/

Frame_renderer::Frame_renderer(FrameBuffer& fb, SdlPresenter& presenter, std::array<Texture, 64>& textures, GameSettings _gameSettings)
	: frameBuffer(fb), presenter(presenter), textures_(textures), gameSettings(_gameSettings)
{}

void Frame_renderer::drawSkybox(const Player& main_pl, int screenWidth, int screenHeight, double FOV) {
	const Texture& skyTex = textures_[5];
	if (skyTex.valid()) {
		// Мы рисуем небо только в верхней половине экрана (над горизонтом)
		const int horizon = screenHeight / 2;

		for (int x = 0; x < screenWidth; ++x) {
			//Вычисляем угол луча для текущего столбца
			double cameraX = 2.0 * x / screenWidth - 1.0;
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
}

void Frame_renderer::drawFloor(const Player& main_pl, int screenWidth, int screenHeight, double projectionDistance, double FOV) {
	// Хотим кароч красивый пол
	const Texture& floorTex = textures_[4];
	const double FOV_half = FOV / 2.0;
	const double leftAngle = main_pl.get_A() - FOV_half;
	const double rightAngle = main_pl.get_A() + FOV_half;

	// Направления крайних лучей экрана
	const double rayDirX0 = std::cos(leftAngle);
	const double rayDirY0 = std::sin(leftAngle);
	const double rayDirX1 = std::cos(rightAngle);
	const double rayDirY1 = std::sin(rightAngle);

	if (floorTex.valid()) {
		// Идем по строкам: от горизонта вниз
		for (int y = (screenHeight / 2) + 1e-9; y < screenHeight; ++y) {
			// Расстояние от горизонта
			//if (screenHeight / 2 - y == 0) continue; // Сама линия горизонта, чтобы не делить на 0

			// Расстояние до пола для этой строки
			double rowDistance = (0.5 * settings::map_scale * projectionDistance) / abs(
				screenHeight / 2 - y);

			double rayDist = rowDistance / std::cos(FOV / 2.0);

			// Шаг текстуры вдоль строки
			double stepX = rayDist * (rayDirX1 - rayDirX0) / screenWidth;
			double stepY = rayDist * (rayDirY1 - rayDirY0) / screenWidth;

			// Начальные координаты в мире для левого края экрана
			double floorX = main_pl.get_X() + rayDist * rayDirX0;
			double floorY = main_pl.get_Y() + rayDist * rayDirY0;

			//затухание в соответствии с расстоянием 
			double brightness = 1.0 - rayDist / (35.0 * settings::map_scale);
			if (brightness < 0.0) brightness = 0.0;
			if (brightness > 1.0) brightness = 1.0;

			//повышаем контрастность
			brightness = std::pow(brightness, 3);

			for (int x = 0; x < screenWidth; x++) {
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
		frameBuffer.fillRect(0, screenHeight / 2, screenWidth, screenHeight / 2,
			Color{ 255, 0, 0, 255 });
	}
}

void Frame_renderer::drawWalls(const Player& main_pl, Playfield& play_field, int screenWidth, int screenHeight,
	int raysCount, double FOV, double projectionDistance) {

	int num_threads = std::thread::hardware_concurrency(); // Узнаем кол-во ядер
	if (num_threads == 0) num_threads = 4;
	//std::cout << "Нашли ядер : " << num_threads << '\n';

	int chunk_size = raysCount / num_threads;
	std::vector<std::future<void>> futures;

	for (int t = 0; t < num_threads; ++t) {
		int start_x = t * chunk_size;
		int end_x = (t == num_threads - 1) ? raysCount : (t + 1) * chunk_size;

		futures.push_back(std::async(std::launch::async, [this, &main_pl, &play_field, start_x, end_x,
			raysCount, FOV, projectionDistance, screenHeight]() {
			for (int i = start_x; i < end_x; i++) {
				//простраиваем луч
				double cameraX = 2.0 * i / raysCount - 1.0; // [-1:1] - коэф от левого до правого края.

				// Истинный угол луча с учетом плоской проекции
				double rayAngle = main_pl.get_A() + std::atan(cameraX * std::tan(FOV / 2.0));

				//double rayAngle = main_pl.get_A() - FOV / 2 + FOV * i / raysCount;


				RayHit hit = RayTracing(main_pl.get_X(), main_pl.get_Y(), rayAngle, main_pl.get_A(), play_field);
				if (!hit.hit) continue;
				depthBuffer_[i] = hit.depth;

				int height = static_cast<int>(settings::map_scale * projectionDistance / hit.depth);
				int yStart = static_cast<int>(screenHeight / 2.0 - height / 2.0);

				//затухание в соответствии с расстоянием 
				double brightness = 1.0 - hit.depth / (35.0 * settings::map_scale);
				if (brightness < 0.0) brightness = 0.0;
				if (brightness > 1.0) brightness = 1.0;

				//повышаем контрастность
				brightness = std::pow(brightness, 3);

				const Texture& tex = textures_[hit.wallId];

				if (!tex.valid()) {
					Color c{ 255, 0, 0, 255 };
					frameBuffer.fillRect(i, yStart, 1, height, c);
					depthBuffer_[i] = hit.depth;
					continue;
				}


				int texX = static_cast<int>(hit.wallU * tex.width());
				if (texX >= tex.width()) texX = tex.width() - 1;

				const double texStep = static_cast<double>(tex.height()) / height;
				const int y0 = std::max(0, yStart);
				const int y1 = std::min(static_cast<int>(screenHeight), yStart + height);
				double texPos = (y0 - yStart) * texStep;

				for (int y = y0; y < y1; ++y) {
					int texY = static_cast<int>(texPos) % tex.height();
					texPos += texStep;
					if ((tex.sample(texX, texY) >> 24) < 128) continue;
					frameBuffer.setPixel(i, y, toColor(tex.sample(texX, texY), brightness));
				}
				depthBuffer_[i] = hit.depth;
			}
			}));
	}
	for (auto& f : futures) f.get(); // Ждем завершения всех потоков
}

void Frame_renderer::drawSprites(const Player& main_pl, Playfield& play_field, double projectionDistance) {
	double c = std::cos(main_pl.get_A());
	double s = std::sin(main_pl.get_A());
	std::ranges::sort(play_field.sprites, [&](const Sprite& a, const Sprite& b) {
		return a.x * c + a.y * s > b.x * c + b.y * s;
		});
	for (const Sprite& sprite : play_field.sprites) {
		const Texture& texture = textures_[sprite.textureId];
		if (!texture.valid()) continue;
		spriteRenderer.draw(
			sprite,
			main_pl,
			frameBuffer,
			depthBuffer_,
			projectionDistance,
			texture
		);
	}
}

void Frame_renderer::render(const Player& main_pl, Playfield& play_field, double timeRemaining, double levelTimeLimit) {

	int screenWidth = frameBuffer.width();
	int screenHeight = frameBuffer.height();

	//угол обзора
	double FOV = gameSettings.fovDegrees() * std::numbers::pi / 180.0;
	//расстояние до плоскости экрана, на который проэцируется все обьекты из точки игрока
	double projectionDistance = screenWidth / (2.0 * std::tan(FOV / 2.0));

	//кол-во лучей на экран
	int raysCount = screenWidth;

	depthBuffer_.assign(
		screenWidth,
		std::numeric_limits<double>::infinity()
	);

	drawSkybox(main_pl, screenWidth, screenHeight, FOV);
	drawFloor(main_pl, screenWidth, screenHeight, projectionDistance, FOV);
	drawWalls(main_pl, play_field, screenWidth, screenHeight, raysCount, FOV, projectionDistance);
	drawSprites(main_pl, play_field, projectionDistance);

	// в последнюю очередь дорисовываем интерфейс 
	draw_interface(frameBuffer, (timeRemaining != 0 ? timeRemaining / levelTimeLimit : 0));

	//готовый кадр отдаём презентеру
	presenter.present(frameBuffer);
}