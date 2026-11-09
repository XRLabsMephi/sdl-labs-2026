#include "wolf/render/SdlPresenter.hpp"

#include <SDL3/SDL.h>

#include <cstdint>

namespace wolf
{
bool SdlPresenter::initialize(SDL_Renderer* renderer, int width, int height)
{
    texture_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    if (!texture_)
    {
        return false;
    }

    SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST);
    return true;
}

bool SdlPresenter::present(SDL_Renderer* renderer, const FrameBuffer& frameBuffer)
{
    if (!texture_)
    {
        return false;
    }

    if (!SDL_UpdateTexture(texture_, nullptr, frameBuffer.pixels().data(), frameBuffer.width() * static_cast<int>(sizeof(std::uint32_t))))
    {
        return false;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    if (!SDL_RenderClear(renderer))
    {
        return false;
    }

    if (!SDL_RenderTexture(renderer, texture_, nullptr, nullptr))
    {
        return false;
    }

    return SDL_RenderPresent(renderer);
}

void SdlPresenter::shutdown()
{
    if (texture_)
    {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
}
}
