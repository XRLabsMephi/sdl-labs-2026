#include "menu_input.h"

int MenuInput::hoveredButton(const MenuScreen& screen, double x, double y) const {
	for (int i = 0; i < static_cast<int>(screen.buttons.size()); i++) {
		if (screen.buttons[i].contains(x, y)) {
			return i;
		}
	}
	return -1;
}

std::optional<ButtonType> MenuInput::handleClick(const SDL_Event& event, const MenuScreen& screen) const {
	if (event.type != SDL_EVENT_MOUSE_BUTTON_DOWN) {
		return std::nullopt;
	}

	if (event.button.button != SDL_BUTTON_LEFT) {
		return std::nullopt;
	}

	int index = hoveredButton(screen, event.button.x, event.button.y);

	if (index == -1) {
		return std::nullopt;
	}

	return screen.buttons[index].getType();
}