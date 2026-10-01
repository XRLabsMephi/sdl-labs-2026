#pragma once
#include "SDL3/SDL.h"
#include "input/PlayerController.hpp"
#include "input/InputState.hpp"
#include "world/Player.hpp"
#include "world/Map.hpp"
#include <iostream>

class Application {
    PlayerController controller;
    Player player;
    Map map;
    SDL_Window* window_ = nullptr;

public:
    Application();
    ~Application();

    void run();
};