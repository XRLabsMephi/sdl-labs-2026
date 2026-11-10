#include "menu_screens.h"

std::vector<MenuScreen> createMenuScreens() {
	return {
	        {
	        	MenuPage::Main, 27, {
					Button(150, 25, 500, 200, ButtonType::Placeholder, 40, 40),
	        		Button(250, 235, 300, 50, ButtonType::Play, 9, 10),
					Button(250, 305, 300, 50, ButtonType::Settings, 13, 14),
					Button(250, 375, 300, 50, ButtonType::Quit, 15, 16)
				}
	        },
			{
				MenuPage::Play, 27, {
					Button(250, 200, 300, 50, ButtonType::Hub, 17, 18),
					Button(250, 270, 300, 50, ButtonType::FirstLevel, 19, 20),
					Button(250, 410, 300, 50, ButtonType::MainMenu, 21, 22)
				}
			},
			{
				MenuPage::Records, 27, {
					Button(250, 410, 300, 50, ButtonType::MainMenu, 21, 22)
				}
			},
			{
				MenuPage::Settings, 27, {
					Button(250, 200, 300, 50, ButtonType::ChangeSensitivity, 30, 31),
					Button(250, 270, 300, 50, ButtonType::ChangeFOV, 36, 37),
					Button(250, 410, 300, 50, ButtonType::MainMenu, 21, 22)
				}
			}
	};
}