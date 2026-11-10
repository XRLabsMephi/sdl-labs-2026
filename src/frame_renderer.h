#pragma once

#include <array>
#include <cmath>
#include <vector>
#include <limits>
#include <numbers>
#include <algorithm>

#include "player.h"
#include "settings.h"
#include "raytracer.h"
#include "playfield.h"
#include "interface.h"
#include "framebuffer.h"
#include "texturestore.h"
#include "sdl_presenter.h"
#include "sprite_renderer.h"
#include "menu/game_settings.h"

/*
	Хотим подготовить кадр к отправке в sdl presenter 
*/

class Frame_renderer {
public:
    Frame_renderer(FrameBuffer& fb, SdlPresenter& presenter, std::array<Texture, 64>& textures_, GameSettings _gameSettings);

    //отрисовать все
    void render(const Player& main_pl, Playfield& play_field, double timeRemaining, double levelTimeLimit);

private:
    FrameBuffer& frameBuffer;
    SdlPresenter& presenter;
    std::array<Texture, 64>& textures_;
    SpriteRenderer spriteRenderer;
    std::vector<double> depthBuffer_;
    GameSettings gameSettings;

    //отрисовать небо
    void drawSkybox(const Player& main_pl, int screenWidth, int screenHeight, double FOV);
    //отрисовать пол
    void drawFloor(const Player& main_pl, int screenWidth, int screenHeight, double projectionDistance, double FOV);
    //отрисовать стены
    void drawWalls(const Player& main_pl, Playfield& play_field, int screenWidth, int screenHeight,
        int raysCount, double FOV, double projectionDistance);
    //отрисовать спрайты
    void drawSprites(const Player& main_pl, Playfield& play_field, double projectionDistance);
};