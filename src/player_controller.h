#pragma once

#include <vector>

#include "framebuffer.h"
#include "sdl_presenter.h"
#include "player.h"
#include "playfield.h"
#include "texturestore.h"
#include "sprite_renderer.h"
#include "interface.h"

/*
    ’отим, чтобы управление персонажем было вынесено в отдельные файлы
    и было скрыто от основного исполн€ющего файла. 
*/

class PlayerController {
public:
    void handle_keyboard(const bool* keys, Player& main_pl, Playfield& play_field, double deltaTime);
    void handle_mouse(const SDL_Event& event, Player& main_pl);
private:
};