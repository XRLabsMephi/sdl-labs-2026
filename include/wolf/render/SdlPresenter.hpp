#pragma once

struct SDL_Renderer;
struct SDL_Texture;

namespace wolf
{

class FrameBuffer;

// Uploads the CPU-rendered frame to an SDL window. The caller owns the renderer
// and must keep it alive until this presenter is destroyed.
class SdlPresenter
{
public:
    explicit SdlPresenter(SDL_Renderer* renderer);
    ~SdlPresenter();

    SdlPresenter(const SdlPresenter&) = delete;
    SdlPresenter& operator=(const SdlPresenter&) = delete;

    void present(const FrameBuffer& frame);

private:
    void ensureTexture(int width, int height);

    SDL_Renderer* renderer_;
    SDL_Texture* texture_ = nullptr;
    int textureWidth_ = 0;
    int textureHeight_ = 0;
};

} // namespace wolf
