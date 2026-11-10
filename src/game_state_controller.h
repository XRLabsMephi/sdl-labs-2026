#pragma once

#include <vector>

#include "player.h"
#include "interface.h"
#include "playfield.h"
#include "framebuffer.h"
#include "texturestore.h"
#include "sdl_presenter.h"
#include "game_director.h"
#include "sprite_renderer.h"

/*
    Хотим иметь класс, который отвечает за изменения состояния игры.
*/

class Game_state_controller {
public:

    //обработать возможное поднятие предметов
    void try_pick_items(Player& main_pl, Playfield& play_field, std::array<Texture, 64>& textures_, double deltaTime);

    double get_time_remaining();
    double get_time_limit();

    void set_time_limit(double _time_limit);

    void load_hub(Player& main_pl, Playfield& play_field, std::array<Texture, 64>& textures_);
    void load_level(Player& main_pl, Playfield& play_field, std::array<Texture, 64>& textures_);
private:
    int cnt = 0;
    bool isTimerRunning = false;
    double timeRemaining = 0.0;
    double LEVEL_TIME_LIMIT = 20.0;
    Director director;
    int state = 1;
};