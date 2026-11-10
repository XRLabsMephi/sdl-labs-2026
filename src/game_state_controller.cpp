#include <cmath>  
#include <array>
#include <iostream>   
#include <algorithm> 

#include "fog.h"
#include "settings.h"
#include "map_generator.h"
#include "game_state_controller.h"

/*
	Файл с реализацией функций из game_state_controller.h
*/

void Game_state_controller::load_hub(Player& main_pl, Playfield& play_field, std::array<Texture, 64>& textures_) {
	play_field.load_map("levels/hub.txt");
	main_pl.set_cords({ 5, 5 });

	main_pl.add_score(-1e9);
	textures_[8].loadFromFile("textures/teleport_lock.bmp");
}

void Game_state_controller::load_level(Player& main_pl, Playfield& play_field, std::array<Texture, 64>& textures_) {
	main_pl.add_score(-1e9);
	cnt++;

	auto cr = director.generate_input();
	auto args = cr.first;
	auto new_map = generate_map(args[0], args[1], args[2], args[3], args[4]);

	save_map_to_file("levels/generated.txt", new_map);

	play_field.load_map("levels/generated.txt");

	play_field.print();
	textures_[8].loadFromFile("textures/teleport_lock.bmp");
	main_pl.set_cords(play_field.getPlayer_coords());

	LEVEL_TIME_LIMIT = cr.second;
	timeRemaining = cr.second;
	isTimerRunning = true;
}

void Game_state_controller::try_pick_items(Player& main_pl, Playfield& play_field, std::array<Texture, 64>& textures_, double deltaTime) {

	// проверили на поднятие
	size_t found = std::erase_if(play_field.sprites, [&](const Sprite& sprite) {
		return sprite.type == SpriteType::food &&
			std::hypot(sprite.x - main_pl.get_X(), sprite.y - main_pl.get_Y())
			< 0.6 * settings::map_scale;
		});

	main_pl.add_score(found);

	if (main_pl.get_score() == play_field.getMax_food()) {
		textures_[8].loadFromFile("textures/teleport_full_unlock.bmp");
	}
	else if (main_pl.get_score() * 2 >= play_field.getMax_food()) {
		textures_[8].loadFromFile("textures/teleport_half_lock.bmp");
	}

	for (auto& teleport : play_field.teleports) {
		if (std::hypot(teleport.x - main_pl.get_X(), teleport.y - main_pl.get_Y())
			< 0.4 * settings::map_scale) {
			if (main_pl.get_score() == play_field.getMax_food()) {
				director.push_result(timeRemaining);
				load_level(main_pl, play_field, textures_);
				int cur = cnt;
				cur %= 10;
				//cur = 8;
				if (state != 3 && cur > 6) {
					state = 3;
					textures_[1].loadFromFile("textures/level_3/wall01.bmp");
					textures_[2].loadFromFile("textures/level_3/wall02.bmp");
					textures_[3].loadFromFile("textures/level_3/wall03.bmp");
					textures_[4].loadFromFile("textures/level_3/floor.bmp");
					textures_[5].loadFromFile("textures/level_3/skybox.bmp");
					textures_[6].loadFromFile("textures/level_3/sprite01.bmp");
					textures_[7].loadFromFile("textures/level_3/sprite02.bmp");
				}
				else if (state != 2 && cur > 3 && cur <= 6) {
					state = 2;
					textures_[1].loadFromFile("textures/level_2/wall01.bmp");
					textures_[2].loadFromFile("textures/level_2/wall02.bmp");
					textures_[3].loadFromFile("textures/level_2/wall03.bmp");
					textures_[4].loadFromFile("textures/level_2/floor.bmp");
					textures_[5].loadFromFile("textures/level_2/skybox.bmp");
					textures_[6].loadFromFile("textures/level_2/sprite01.bmp");
					textures_[7].loadFromFile("textures/level_2/sprite02.bmp");
				}
				else if (state != 1 && cur <= 3) {
					state = 1;
					textures_[1].loadFromFile("textures/level_1/wall01.bmp");
					textures_[2].loadFromFile("textures/level_1/wall02.bmp");
					textures_[3].loadFromFile("textures/level_1/wall03.bmp");
					textures_[4].loadFromFile("textures/level_1/floor.bmp");
					textures_[5].loadFromFile("textures/level_1/skybox.bmp");
					textures_[6].loadFromFile("textures/level_1/sprite01.bmp");
					textures_[7].loadFromFile("textures/level_1/sprite02.bmp");
				}
			}
			else {
				//std::cout << "You collected " << main_pl.get_score() << " food out of " << play_field.getMax_food() << '\n';
			}
		}
	}

	if (isTimerRunning) {
		timeRemaining -= deltaTime;

		if (timeRemaining <= 0.0) {
			timeRemaining = 0.0;
			isTimerRunning = false;

			std::cout << "gg, you lost, your score : " << cnt << '\n';

			state = 1;
			textures_[1].loadFromFile("textures/level_1/wall01.bmp");
			textures_[2].loadFromFile("textures/level_1/wall02.bmp");
			textures_[3].loadFromFile("textures/level_1/wall03.bmp");
			textures_[4].loadFromFile("textures/level_1/floor.bmp");
			textures_[5].loadFromFile("textures/level_1/skybox.bmp");
			textures_[6].loadFromFile("textures/level_1/sprite01.bmp");
			textures_[7].loadFromFile("textures/level_1/sprite02.bmp");

			load_hub(main_pl, play_field, textures_);
		}
	}
}

double Game_state_controller::get_time_remaining() { return timeRemaining; }
double Game_state_controller::get_time_limit() { return LEVEL_TIME_LIMIT; }

void Game_state_controller::set_time_limit(double _time_limit) {
	LEVEL_TIME_LIMIT = std::min(60.0, std::max(10.0, _time_limit));
}