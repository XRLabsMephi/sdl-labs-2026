#ifndef SOFTWAREGRAPHICSLABS_RENDER_SDL_PRESENTER_H_
#define SOFTWAREGRAPHICSLABS_RENDER_SDL_PRESENTER_H_

#include "frame_buffer.h"


struct SDL_Renderer;
struct SDL_Texture;

class SdlPresenter {
public:

    SdlPresenter(SDL_Renderer* renderer, int width, int height);
    ~SdlPresenter();

    // Заливает FrameBuffer в текстуру и выводит на экран
    void Present(const FrameBuffer& frame_buffer);

private:
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
};

#endif