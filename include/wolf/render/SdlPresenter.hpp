#pragma once
#include <SDL3/SDL.h>
#include "wolf/render/FrameBuffer.hpp"

namespace wolf::render {
    class SdlPresenter {
    public:
        //для 2д рендеринга
        SdlPresenter(SDL_Renderer* renderer, int width, int height);

        ~SdlPresenter();

        SdlPresenter(const SdlPresenter&) = delete;
        SdlPresenter& operator=(const SdlPresenter&) = delete;

        void present(const FrameBuffer& frameBuffer);

    private:
        SDL_Renderer* renderer = nullptr;
        SDL_Texture* texture = nullptr;
    };
}