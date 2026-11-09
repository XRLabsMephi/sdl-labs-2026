#include "wolf/render/SdlPresenter.hpp"

#include "wolf/render/FrameBuffer.hpp"

#include <SDL3/SDL.h>

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

namespace wolf
{
namespace
{

[[nodiscard]] std::runtime_error sdlError(const char* operation)
{
    return std::runtime_error(std::string(operation) + ": " + SDL_GetError());
}

} // namespace

SdlPresenter::SdlPresenter(SDL_Renderer* renderer)
    : renderer_(renderer)
{
    if (renderer_ == nullptr)
    {
        throw std::invalid_argument("SdlPresenter requires an SDL renderer");
    }
}

SdlPresenter::~SdlPresenter()
{
    if (texture_ != nullptr)
    {
        SDL_DestroyTexture(texture_);
    }
}

void SdlPresenter::ensureTexture(int width, int height)
{
    if (texture_ != nullptr && textureWidth_ == width && textureHeight_ == height)
    {
        return;
    }

    SDL_Texture* next = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STREAMING, width, height);
    if (next == nullptr)
    {
        throw sdlError("SDL_CreateTexture");
    }
    if (!SDL_SetTextureScaleMode(next, SDL_SCALEMODE_NEAREST))
    {
        SDL_DestroyTexture(next);
        throw sdlError("SDL_SetTextureScaleMode");
    }

    if (texture_ != nullptr)
    {
        SDL_DestroyTexture(texture_);
    }
    texture_ = next;
    textureWidth_ = width;
    textureHeight_ = height;
}

void SdlPresenter::present(const FrameBuffer& frame)
{
    if (frame.width() <= 0 || frame.height() <= 0
        || frame.width() > std::numeric_limits<int>::max() / static_cast<int>(sizeof(std::uint32_t)))
    {
        throw std::invalid_argument("FrameBuffer has invalid dimensions for SDL");
    }

    ensureTexture(frame.width(), frame.height());

    if (!SDL_UpdateTexture(texture_, nullptr, frame.data(),
                           frame.width() * static_cast<int>(sizeof(std::uint32_t))))
    {
        throw sdlError("SDL_UpdateTexture");
    }
    if (!SDL_SetRenderTarget(renderer_, nullptr))
    {
        throw sdlError("SDL_SetRenderTarget");
    }
    if (!SDL_RenderClear(renderer_))
    {
        throw sdlError("SDL_RenderClear");
    }
    if (!SDL_RenderTexture(renderer_, texture_, nullptr, nullptr))
    {
        throw sdlError("SDL_RenderTexture");
    }
    if (!SDL_RenderPresent(renderer_))
    {
        throw sdlError("SDL_RenderPresent");
    }
}

} // namespace wolf
