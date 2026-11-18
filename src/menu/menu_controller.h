#pragma once

#include <vector>
#include "main_menu.h"

class MenuController {
public:
	MenuController(std::vector<MenuScreen> screens, GameSettings& settings);

	bool handleEvent(const SDL_Event& event, SDL_Window* window);

	bool isOpen() const;
	MenuAction takeAction();

	void render(
		FrameBuffer& frameBuffer,
		std::span<const Texture> textures
	) const;

private:
	MainMenu mainMenu_;
	bool active_ = true;
	MenuAction pendingAction_ = MenuAction::None;
};