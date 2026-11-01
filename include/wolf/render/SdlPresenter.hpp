#pragma once

#include <SDL3/SDL.h>

class FrameBuffer;

class SdlPresenter {
public:
    SdlPresenter(int width, int height);
    ~SdlPresenter();

    SdlPresenter(const SdlPresenter&) = delete;
    SdlPresenter& operator=(const SdlPresenter&) = delete;

    void present(const FrameBuffer& frame);

private:
    void release();

    int width_;
    int height_;
    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;
};