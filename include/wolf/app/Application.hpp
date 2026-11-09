#pragma once

#include "wolf/input/PlayerController.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SdlPresenter.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

struct SDL_Window;
struct SDL_Renderer;

namespace wolf
{
class Application
{
public:
    int run();

private:
    bool initialize();
    void shutdown();
    InputState readInput(bool& running);
    void updateTitle();

    static constexpr int SCREEN_W = 1120;
    static constexpr int SCREEN_H = 640;

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;

    Map map_;
    Player player_;
    PlayerController controller_;
    FrameBuffer frameBuffer_{SCREEN_W, SCREEN_H};
    Raycaster raycaster_;
    SdlPresenter presenter_;
};
}
