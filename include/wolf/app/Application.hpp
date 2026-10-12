#pragma once
#include "SDL3/SDL.h"
#include "input/PlayerController.hpp"
#include "input/InputState.hpp"
#include "world/Player.hpp"
#include "world/Map.hpp"
#include "render/SDLPresenter.hpp"
#include "render/FrameBuffer.hpp"
#include "raycast/RayCaster.hpp"
#include <memory>
#include <iostream>

class Application {
    PlayerController controller;
    Player player;
    Map map;

    std::unique_ptr<SDLPresenter> presenter;
    std::unique_ptr<FrameBuffer> buffer;
    RayCaster raycaster;

public:
    Application();
    ~Application() = default;

    void run();
};