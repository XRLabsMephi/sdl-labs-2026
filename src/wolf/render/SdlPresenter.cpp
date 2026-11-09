#include "wolf/render/SdlPresenter.hpp"

namespace wolf::render {

    SdlPresenter::SdlPresenter(SDL_Renderer* renderer, int width, int height) : renderer(renderer) {
        texture = SDL_CreateTexture( renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            width,height
            );
    } //ARGB8888 херня для 32 битного кодирования пикселя
    //стриминг текстур - текстура будет часто меняться => её нужно будет блокировать для записи

    SdlPresenter::~SdlPresenter() {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }

    void SdlPresenter::present(const FrameBuffer& frameBuffer) {
        //пиксели из CPU FrameBuffer в текстуру GPU
        int pitch = frameBuffer.getWidth() * sizeof(std::uint32_t);
        SDL_UpdateTexture(texture, nullptr, frameBuffer.getPixels(), pitch);
        
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }
}