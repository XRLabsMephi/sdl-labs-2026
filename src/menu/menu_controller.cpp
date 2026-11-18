#include "menu_controller.h"
#include "../settings.h"

#include <utility>

MenuController::MenuController(std::vector<MenuScreen> screens, GameSettings& settings) : mainMenu_(
	std::move(screens), settings) {
}

bool MenuController::isOpen() const {
	return active_;
}

MenuAction MenuController::takeAction() {
	MenuAction result = pendingAction_;
	pendingAction_ = MenuAction::None;
	return result;
}

bool MenuController::handleEvent(const SDL_Event& event, SDL_Window* window) {
	if (!active_) {
		if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE && !event.key.repeat) {
			active_ = true;
			mainMenu_.open();
			SDL_SetWindowRelativeMouseMode(window, false);
			return true;
		}
		return false;
	}

	SDL_Event adjusted = event;

	int windowWidth, windowHeight, pixelWidth, pixelHeight;
	if (SDL_GetWindowSize(window, &windowWidth, &windowHeight) &&
		SDL_GetWindowSizeInPixels(window, &pixelWidth, &pixelHeight) &&
		windowWidth > 0 && windowHeight > 0) {
		double scaleX = static_cast<double>(pixelWidth) / windowWidth;
		double scaleY = static_cast<double>(pixelHeight) / windowHeight;
		double dx = (pixelWidth - settings::screenWidth) / 2.0;
		double dy = (pixelHeight - settings::screenHeight) / 2.0;

		if (event.type == SDL_EVENT_MOUSE_MOTION) {
			adjusted.motion.x = event.motion.x * scaleX - dx;
			adjusted.motion.y = event.motion.y * scaleY - dy;
		}

		if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
			adjusted.button.x = event.button.x * scaleX - dx;
			adjusted.button.y = event.button.y * scaleY - dy;
		}
	}
	MenuAction action = mainMenu_.handleEvent(adjusted);

	if (action == MenuAction::StartHub || action == MenuAction::StartRoguelike) {
		active_ = false;
		SDL_SetWindowRelativeMouseMode(window, true);
	}

	if (action != MenuAction::None && pendingAction_ == MenuAction::None) {
		pendingAction_ = action;
	}

	return true;
}

void MenuController::render(FrameBuffer& frameBuffer, std::span<const Texture> textures) const {
	mainMenu_.render(frameBuffer, textures);
}
