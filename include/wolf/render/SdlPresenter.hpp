#pragma once

struct SDL_Renderer;
struct SDL_Texture;

namespace wolf {

class FrameBuffer;

class SdlPresenter {
public:
    SdlPresenter::SdlPresenter(SDL_Renderer* renderer, int width, int height)
        : renderer_(renderer), width_(width), height_(height) {
        texture_ = SDL_CreateTexture(
            renderer_,
            SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_STREAMING,
            width_,
            height_
        );

        if (texture_ &&
            !SDL_SetTextureScaleMode(texture_, SDL_SCALEMODE_NEAREST)) {
            SDL_DestroyTexture(texture_);
            texture_ = nullptr;
        }
    }
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
