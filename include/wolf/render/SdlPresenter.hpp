#pragma once

struct SDL_Renderer;
struct SDL_Texture;

namespace wolf {

class FrameBuffer;

class SdlPresenter {
public:
    SdlPresenter(SDL_Renderer* renderer, int width, int height);
    ~SdlPresenter();

    SdlPresenter(const SdlPresenter&) = delete;
    SdlPresenter& operator=(const SdlPresenter&) = delete;

    bool valid() const { return texture_ != nullptr; }
    bool present(const FrameBuffer& frame);

private:
    SDL_Renderer* renderer_;
    SDL_Texture* texture_ = nullptr;
    int width_, height_;
};

}
