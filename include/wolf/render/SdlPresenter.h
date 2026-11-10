#pragma once
#include <SDL3/SDL.h>
#include "SDL3/SDL_render.h"
#include <memory>

#include "FrameBuffer.h"

namespace render {
    class SdlPresenter {
        struct SDL_Renderer_Deleter {
            void operator()(SDL_Renderer* renderer) const noexcept;
        };

        struct SDL_Texture_Deleter {
            void operator()(SDL_Texture* texture) const noexcept;
        };

        struct SDL_Window_Deleter {
            void operator()(SDL_Window* window) const noexcept;
        };

        struct SDL_Surface_Deleter {
            void operator()(SDL_Surface* surface) const noexcept;
        };

        void fail(std::string_view message);

        std::unique_ptr<SDL_Renderer, SDL_Renderer_Deleter> renderer_;
        std::unique_ptr<SDL_Texture, SDL_Texture_Deleter> texture_;
        std::unique_ptr<SDL_Window, SDL_Window_Deleter> window_;
        std::unique_ptr<SDL_Surface, SDL_Surface_Deleter> surface_;
        buf_size_t width_ = 0, height_ = 0;
        std::string error_message;

    public:
        /**
         * @brief initializes Sdl_Presenter to draw to given window
         * @param window Window that image will be rendered to
         */
        explicit SdlPresenter(SDL_Window* window);

        /**
         * @brief Prints FrameBuffer into Texture and draws it on the screen
         * @param buffer Buffer to get pixels from
         * @return @c true on success, @c false on error and set an error message to be read from getError
         */
        bool present(const FrameBuffer& buffer);

        std::string getError();
    };

}
