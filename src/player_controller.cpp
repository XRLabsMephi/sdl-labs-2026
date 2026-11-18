#include <SDL3/SDL.h>

#include "player_controller.h"
/*
	Файл с реализацией функций из player_controller.h
*/

void PlayerController::handle_keyboard(const bool* keys, Player& main_pl, Playfield& play_field, double deltaTime) {
	if (keys[SDL_SCANCODE_A]) {
		main_pl.left(play_field, deltaTime);
		if (keys[SDL_SCANCODE_LSHIFT]) main_pl.left(play_field, deltaTime);
	}
	if (keys[SDL_SCANCODE_D]) {
		main_pl.right(play_field, deltaTime);
		if (keys[SDL_SCANCODE_LSHIFT]) main_pl.right(play_field, deltaTime);
	}
	if (keys[SDL_SCANCODE_W]) {
		main_pl.forward(play_field, deltaTime);
		if (keys[SDL_SCANCODE_LSHIFT]) main_pl.forward(play_field, deltaTime);
	}
	if (keys[SDL_SCANCODE_S]) {
		main_pl.back(play_field, deltaTime);
		if (keys[SDL_SCANCODE_LSHIFT]) main_pl.back(play_field, deltaTime);
	}

	if (keys[SDL_SCANCODE_LEFT]) {
		main_pl.turn_left(deltaTime);
	}

	if (keys[SDL_SCANCODE_RIGHT]) {
		main_pl.turn_right(deltaTime);
	}
}

void PlayerController::handle_mouse(const SDL_Event& event, Player& main_pl) {
	if (event.type == SDL_EVENT_MOUSE_MOTION) {
		const double mouseSensitivity = 0.002; 
		main_pl.turn_right(event.motion.xrel * mouseSensitivity);
	}
}