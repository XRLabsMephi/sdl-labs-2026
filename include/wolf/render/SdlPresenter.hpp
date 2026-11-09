#pragma once

#include "wolf/render/FrameBuffer.hpp"

struct SDL_Renderer;
struct SDL_Texture;

namespace wolf
{
class SdlPresenter
{
public:
    bool initialize(SDL_Renderer* renderer, int width, int height);
    bool present(SDL_Renderer* renderer, const FrameBuffer& frameBuffer);
    void shutdown();

private:
    SDL_Texture* texture_ = nullptr;
};
}
