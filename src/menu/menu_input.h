#pragma once

#include <SDL3/SDL.h>
#include <optional>
#include "menu_screen.h"

class MenuInput {
public:
	int hoveredButton(const MenuScreen& screen, double x, double y) const;

	std::optional<ButtonType> handleClick(const SDL_Event& event, const MenuScreen& screen) const;
};