#include "main_menu.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

MainMenu::MainMenu(std::vector<MenuScreen> screens, GameSettings& settings) : screens_(std::move(screens)),
                                                                              settings_(settings) {
	if (!changeScreen(MenuPage::Main)) throw std::invalid_argument("Main menu screen not found");
	updateSettingsButtons();
}

MenuPage MainMenu::getPage() const {
	return currentPage_;
}

void MainMenu::open() {
	currentPage_ = MenuPage::Main;
	mouseX_ = -1;
	mouseY_ = -1;
}

const MenuScreen& MainMenu::currentScreen() const {
	auto it = std::find_if(
		screens_.begin(),
		screens_.end(),
		[&](const MenuScreen& screen) {
			return screen.page == currentPage_;
		}
	);

	if (it == screens_.end()) {
		throw std::logic_error("Current menu screen not found");
	}

	return *it;
}

bool MainMenu::changeScreen(MenuPage page) {
	auto it = std::find_if(
		screens_.begin(),
		screens_.end(),
		[&](const MenuScreen& screen) {
			return screen.page == page;
		}
	);

	if (it == screens_.end()) {
		return false;
	}

	currentPage_ = page;
	return true;
}

MenuAction MainMenu::handleEvent(const SDL_Event& event) {
	if (event.type == SDL_EVENT_MOUSE_MOTION) {
		mouseX_ = event.motion.x;
		mouseY_ = event.motion.y;
	}

	if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
		mouseX_ = event.button.x;
		mouseY_ = event.button.y;
	}

	if (event.type == SDL_EVENT_KEY_DOWN &&
		event.key.key == SDLK_ESCAPE) {
		open();
		return MenuAction::None;
	}

	auto clicked = input_.handleClick(event, currentScreen());

	if (!clicked.has_value()) {
		return MenuAction::None;
	}

	switch (*clicked) {
	case ButtonType::MainMenu:
		changeScreen(MenuPage::Main);
		break;

	case ButtonType::Play:
		changeScreen(MenuPage::Play);
		break;

	case ButtonType::Records:
		changeScreen(MenuPage::Records);
		break;

	case ButtonType::Settings:
		changeScreen(MenuPage::Settings);
		break;

	case ButtonType::Hub:
		return MenuAction::StartHub;

	case ButtonType::FirstLevel:
		return MenuAction::StartRoguelike;

	case ButtonType::Quit:
		return MenuAction::Quit;

	case ButtonType::ChangeSensitivity:
		settings_.sensitivityLevel = (settings_.sensitivityLevel + 1) % 3;
		updateSettingsButtons();
		break;

	case ButtonType::ChangeFOV:
		settings_.fovLevel = (settings_.fovLevel + 1) % 3;
		updateSettingsButtons();
		break;
	}


	return MenuAction::None;
}

void MainMenu::render(FrameBuffer& frameBuffer, std::span<const Texture> textures) const {
	const MenuScreen& screen = currentScreen();

	int hovered = input_.hoveredButton(screen, mouseX_, mouseY_);

	renderer_.render(screen, frameBuffer, textures, hovered);
}

void MainMenu::updateSettingsButtons() {
	for (auto& screen : screens_) {
		if (screen.page != MenuPage::Settings) continue;

		for (auto& button : screen.buttons) {
			if (button.getType() == ButtonType::ChangeSensitivity) {
				int id = 28 + settings_.sensitivityLevel * 2;
				button.setTextures(id, id + 1);
			}

			if (button.getType() == ButtonType::ChangeFOV) {
				int id = 34 + settings_.fovLevel * 2;
				button.setTextures(id, id + 1);
			}
		}
	}
}