#pragma once

#include <vector>
#include "menu_button.h"

enum class MenuPage {
	Main,
	Play,
	Records,
	Settings
};

struct MenuScreen {
	MenuPage page;
	int backgroundTextureId;
	std::vector<Button> buttons;
};