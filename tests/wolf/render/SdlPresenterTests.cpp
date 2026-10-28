#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SdlPresenter.hpp"

#include <SDL3/SDL.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

struct SdlQuit {
    ~SdlQuit() { SDL_Quit(); }
};

void testPresentation() {
    const SdlQuit cleanup;
    require(SDL_Init(SDL_INIT_VIDEO), "SDL video initialization failed");
    const std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window(
        SDL_CreateWindow("Software rendering test", 64, 48, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE),
        SDL_DestroyWindow);
    require(window != nullptr, "SDL window creation failed");

    wolf::FrameBuffer frame(64, 48);
    frame.clear(wolf::makeColor(12, 34, 56));
    wolf::SdlPresenter presenter(*window, frame.width(), frame.height());
    presenter.present(frame);

    require(SDL_SetWindowSize(window.get(), 100, 80), "Window resize failed");
    SDL_PumpEvents();
    presenter.present(frame);
    int logicalWidth = 0, logicalHeight = 0;
    SDL_RendererLogicalPresentation mode{};
    require(SDL_GetRenderLogicalPresentation(SDL_GetRenderer(window.get()),
                &logicalWidth, &logicalHeight, &mode) &&
                logicalWidth == 64 && logicalHeight == 48 &&
                mode == SDL_LOGICAL_PRESENTATION_LETTERBOX,
            "Resize must preserve the logical frame dimensions and aspect ratio");
    frame.setPixel(10, 20, wolf::makeColor(200, 30, 70));
    presenter.present(frame);

    bool mismatched = false;
    try {
        wolf::FrameBuffer wrongSize(1, 1);
        presenter.present(wrongSize);
    } catch (const std::invalid_argument&) {
        mismatched = true;
    }
    require(mismatched, "SdlPresenter must reject mismatched frame sizes");

    bool invalid = false;
    try {
        wolf::SdlPresenter bad(*window, 0, 48);
    } catch (const std::invalid_argument&) {
        invalid = true;
    }
    require(invalid, "SdlPresenter must reject invalid dimensions");
}

} // namespace

int main() {
    try {
        testPresentation();
        std::cout << "All SdlPresenter tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << " | SDL: " << SDL_GetError() << '\n';
        return 1;
    }
}
