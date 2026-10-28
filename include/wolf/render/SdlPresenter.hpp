#pragma once

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace wolf {

    class FrameBuffer;

    class SdlPresenter {
        public:
            SdlPresenter(SDL_Window& window, int width, int height);
            ~SdlPresenter();

            SdlPresenter(const SdlPresenter&) = delete;
            SdlPresenter& operator=(const SdlPresenter&) = delete;
            SdlPresenter(SdlPresenter&&) = delete;
            SdlPresenter& operator=(SdlPresenter&&) = delete;

            void present(const FrameBuffer& frame);

        private:
            SDL_Renderer* renderer_ = nullptr;
            SDL_Texture* texture_ = nullptr;
            int width_;
            int height_;
    };

}
