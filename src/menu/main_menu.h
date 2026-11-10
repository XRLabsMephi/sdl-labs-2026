#pragma once

#include <vector>
#include <span>
#include <SDL3/SDL.h>

#include "menu_screen.h"
#include "menu_input.h"
#include "menu_renderer.h"
#include "game_settings.h"

enum class MenuAction {
	None,
	StartHub,
	StartRoguelike,
	Quit
};

class MainMenu {
public:
	MainMenu(std::vector<MenuScreen> screens, GameSettings& settings);

	MenuAction handleEvent(const SDL_Event& event);

	void render(
		FrameBuffer& frameBuffer,
		std::span<const Texture> textures
	) const;

	void open();
	MenuPage getPage() const;

private:
	const MenuScreen& currentScreen() const;
	bool changeScreen(MenuPage page);

	std::vector<MenuScreen> screens_;
	MenuPage currentPage_ = MenuPage::Main;

	MenuInput input_;
	MenuRenderer renderer_;

	double mouseX_ = -1;
	double mouseY_ = -1;

	GameSettings& settings_;
	void updateSettingsButtons();
};

