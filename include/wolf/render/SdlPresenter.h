#pragma once
#include <SDL3/SDL.h>
#include "SDL3/SDL_render.h"
#include "FrameBuffer.h"

namespace render {
    class SdlPresenter {
        SDL_Texture texture_; // final frame (can be moved into void present(FrameBuffer* buffer) later)
        SDL_Renderer renderer_; //rendering context
    public:

        SdlPresenter(uint32_t width, uint32_t height); //constructor also initializes FrameBuffer so we have a single buffer per program
        void present(FrameBuffer* buffer); // a function that converts FrameBuffer into Texture and draws it on the screen
    };

}
