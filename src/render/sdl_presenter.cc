#include "sdl_presenter.h"

#include <SDL3/SDL.h>
#include <stdexcept>

SdlPresenter::SdlPresenter(SDL_Renderer* renderer, int width, int height)
    : renderer_(renderer) {
    if (!renderer_) {
        throw std::invalid_argument("SdlPresenter received null SDL_Renderer");
    }

    // Создаем потоковую текстуру формата RGBA8888
    texture_ = SDL_CreateTexture(renderer_,
                                 SDL_PIXELFORMAT_RGBA8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 width, height);
    if (!texture_) {
        throw std::runtime_error("Failed to create SDL Texture");
    }
}

SdlPresenter::~SdlPresenter() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
    }
}

void SdlPresenter::Present(const FrameBuffer& frame_buffer) {
    // Копируем массив пикселей (uint32_t) из FrameBuffer в текстуру SDL
    // Pitch (шаг строки) = ширина * 4 байта (размер uint32_t)
    int pitch = frame_buffer.GetWidth() * sizeof(uint32_t);
    SDL_UpdateTexture(texture_, nullptr, frame_buffer.GetPixelData(), pitch);

    SDL_RenderClear(renderer_);

    // Растягиваем/копируем нашу текстуру на весь экран (в SDL3 используется SDL_RenderTexture)
    SDL_RenderTexture(renderer_, texture_, nullptr, nullptr);

    SDL_RenderPresent(renderer_);
}