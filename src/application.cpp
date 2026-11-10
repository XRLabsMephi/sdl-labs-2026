#include <cmath>
#include <limits>
#include <numbers>
#include <iostream>
#include <algorithm>
#include <SDL3/SDL.h>

#include "settings.h"
#include "playfield.h"
#include "framebuffer.h"
#include "application.h"
#include "sdl_presenter.h"
#include "frame_renderer.h"
#include "sprite_renderer.h"
#include "menu/menu_screens.h"
#include "player_controller.h"
#include "menu/game_settings.h" 
#include "menu/menu_controller.h"
#include "game_state_controller.h"

Application::Application() = default;

Application::~Application() {
	if (renderer) {
		SDL_DestroyRenderer(renderer);
		renderer = nullptr;
	}
	if (window) {
		SDL_DestroyWindow(window);
		window = nullptr;
	}
	SDL_Quit();
}

bool Application::launch_SDL() {
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		std::cout << SDL_GetError() << '\n';
		return false;
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
		return false;
	}
	if (!SDL_SetWindowRelativeMouseMode(window, true)) {
		std::cerr << SDL_GetError() << '\n';
	}
	SDL_SetWindowResizable(window, true);
	return true;
}

void Application::close_SDL() {
	SDL_DestroyRenderer(renderer);
	renderer = nullptr;
	SDL_DestroyWindow(window);
	window = nullptr;
	SDL_Quit();
}

void Application::load_textures() {
	textures_[1].loadFromFile("textures/level_1/wall01.bmp");
	textures_[2].loadFromFile("textures/level_1/wall02.bmp");
	textures_[3].loadFromFile("textures/level_1/wall03.bmp");
	textures_[4].loadFromFile("textures/level_1/floor.bmp");
	textures_[5].loadFromFile("textures/level_1/skybox.bmp");
	textures_[6].loadFromFile("textures/level_1/sprite01.bmp");
	textures_[7].loadFromFile("textures/level_1/sprite02.bmp");
	textures_[8].loadFromFile("textures/teleport_lock.bmp");
	//textures_[8].loadFromFile("textures/teleport_half_lock.bmp");
	//textures_[8].loadFromFile("textures/teleport_full_unlock.bmp");

	textures_[9].loadFromFile("textures/play.bmp");
	textures_[10].loadFromFile("textures/play_hover.bmp");
	textures_[11].loadFromFile("textures/records.bmp");
	textures_[12].loadFromFile("textures/records_hover.bmp");
	textures_[13].loadFromFile("textures/settings.bmp");
	textures_[14].loadFromFile("textures/settings_hover.bmp");
	textures_[15].loadFromFile("textures/quit.bmp");
	textures_[16].loadFromFile("textures/quit_hover.bmp");
	textures_[17].loadFromFile("textures/hub.bmp");
	textures_[18].loadFromFile("textures/hub_hover.bmp");
	textures_[19].loadFromFile("textures/first_level.bmp");
	textures_[20].loadFromFile("textures/first_level_hover.bmp");
	textures_[21].loadFromFile("textures/back.bmp");
	textures_[22].loadFromFile("textures/back_hover.bmp");
	textures_[23].loadFromFile("textures/statistics.bmp");
	textures_[24].loadFromFile("textures/statistics_hover.bmp");
	textures_[25].loadFromFile("textures/main_menu.bmp");
	textures_[26].loadFromFile("textures/main_menu_hover.bmp");
	textures_[27].loadFromFile("textures/menu_panorama.bmp");
	textures_[28].loadFromFile("textures/sensitivity_low.bmp");
	textures_[29].loadFromFile("textures/sensitivity_low_hover.bmp");
	textures_[30].loadFromFile("textures/sensitivity_normal.bmp");
	textures_[31].loadFromFile("textures/sensitivity_normal_hover.bmp");
	textures_[32].loadFromFile("textures/sensitivity_high.bmp");
	textures_[33].loadFromFile("textures/sensitivity_high_hover.bmp");
	textures_[34].loadFromFile("textures/fov_70.bmp");
	textures_[35].loadFromFile("textures/fov_70_hover.bmp");
	textures_[36].loadFromFile("textures/fov_90.bmp");
	textures_[37].loadFromFile("textures/fov_90_hover.bmp");
	textures_[38].loadFromFile("textures/fov_110.bmp");
	textures_[39].loadFromFile("textures/fov_110_hover.bmp");
	textures_[40].loadFromFile("textures/mouse_defender.bmp");
}

void Application::load_hub(Playfield& play_field) {
	play_field.load_map("levels/hub.txt");
	play_field.print();
}


int Application::run() {

	//инициализируем SDL
	if (!launch_SDL()) {
		return 1;
	}
	//загружаем текстуры
	load_textures();
	//загружаем хаб
	Playfield play_field(1, 1, settings::map_scale);


	FrameBuffer frameBuffer(settings::screenWidth, settings::screenHeight);
	SdlPresenter presenter(renderer, settings::screenWidth, settings::screenHeight);
	SpriteRenderer spriteRenderer;
	GameSettings gameSettings; // создание настроек 
	MenuController menu(createMenuScreens(), gameSettings); // создание меню

	bool running = true;

	auto [x, y] = play_field.get_player_coords();
	Player main_pl(
		(x + 0.5) * settings::map_scale,
		(y + 0.5) * settings::map_scale,
		-std::numbers::pi / 2,
		0.2
	);

	Uint64 previousTime = SDL_GetTicksNS();

	Frame_renderer frameRenderer(frameBuffer, presenter, textures_, gameSettings);
	PlayerController playerController;
	Game_state_controller gameStateController;

	while (running) {
		Uint64 currentTime = SDL_GetTicksNS();
		double deltaTime =
			static_cast<double>(currentTime - previousTime) / 1'000'000'000.0;
		previousTime = currentTime;
		deltaTime = std::min(deltaTime, 0.05);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) {
				running = false;
			}
			
			playerController.handle_mouse(event, main_pl);

			if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
				int width, height;
				if (SDL_GetWindowSizeInPixels(window, &width, &height)
					&& width > 0 && height > 0) {
					if (width != frameBuffer.width() ||
						height != frameBuffer.height()) {
						if (presenter.resize(width, height)) {
							frameBuffer = FrameBuffer(width, height);
						}
						else {
							std::cerr << SDL_GetError() << '\n';
							running = false;
						}
					}
				}
			}

			if (menu.handleEvent(event, window)) continue; // передача событий в меню
		}

		MenuAction action = menu.takeAction();
		if (action == MenuAction::Quit) {
			running = false; // обработка выхода из игры
		}
		// Обработка попадания в хаб
		if (action == MenuAction::StartHub) {
			gameStateController.load_hub(main_pl, play_field, textures_);
		}
		// Обработка попадания в уровень
		if (action == MenuAction::StartRoguelike) {
			gameStateController.load_level(main_pl, play_field, textures_);
		}

		//#### //#### //#### //#### //#### //#### //#### //#### //#### //#### //####

		if (!running) break;


		//#### //#### //#### //#### //#### //#### //#### //#### //#### //#### //####

		// рендер меню
		if (menu.isOpen()) {
			menu.render(frameBuffer, textures_);
			presenter.present(frameBuffer);
			SDL_Delay(16);
			continue;
		}

		const bool* keys = SDL_GetKeyboardState(nullptr);

		//попытаемся подобрать хотя бы что-то
		gameStateController.try_pick_items(main_pl, play_field, textures_,deltaTime);
		//перемещение
		playerController.handle_keyboard(keys, main_pl, play_field, deltaTime);
		//отрисовать нынешний кадр
		frameRenderer.render(main_pl, play_field, gameStateController.get_time_remaining(), gameStateController.get_time_limit());
	}

	//закругляемся
	close_SDL();

	return 0;
}
