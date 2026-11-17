#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <memory>
#include <stdexcept>
#include <string>

#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SdlPresenter.hpp"

namespace {

void check(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void expectPixel(SDL_Surface* surface, int x, int y, wolf::Color color) {
    Uint8 r = 0, g = 0, b = 0, a = 0;

    check(
        SDL_ReadSurfacePixel(surface, x, y, &r, &g, &b, &a),
        std::string("SDL_ReadSurfacePixel: ") + SDL_GetError()
    );

    check(
        (wolf::Color{r, g, b, a} == color),
        "Incorrect pixel at (" + std::to_string(x) +
        ", " + std::to_string(y) + ")"
    );
}

}

int main(int, char*[]) {
    constexpr int W = 4;
    constexpr int H = 3;

    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(
        SDL_CreateSurface(W, H, SDL_PIXELFORMAT_RGBA32),
        SDL_DestroySurface
    );

    check(surface != nullptr, SDL_GetError());

    std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)> renderer(
        SDL_CreateSoftwareRenderer(surface.get()),
        SDL_DestroyRenderer
    );

    check(renderer != nullptr, SDL_GetError());

    wolf::FrameBuffer frame(W, H);
    wolf::SdlPresenter presenter(renderer.get(), W, H);

    check(presenter.valid(), SDL_GetError());

    const wolf::Color background{30, 40, 50};
    const wolf::Color red{255, 0, 0};
    const wolf::Color green{0, 255, 0};
    const wolf::Color blue{0, 0, 255};

    frame.clear(background);
    frame.setPixel(0, 0, red);
    frame.setPixel(1, 1, green);
    frame.setPixel(W - 1, H - 1, blue);

    check(presenter.present(frame), SDL_GetError());

    expectPixel(surface.get(), 0, 0, red);
    expectPixel(surface.get(), 1, 1, green);
    expectPixel(surface.get(), W - 1, H - 1, blue);
    expectPixel(surface.get(), 2, 0, background);

    wolf::FrameBuffer wrongSize(W - 1, H);

    check(
        !presenter.present(wrongSize),
        "Mismatched frame size was accepted"
    );

    const wolf::Color yellow{255, 255, 0};

    frame.clear(yellow);
    check(presenter.present(frame), SDL_GetError());

    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            expectPixel(surface.get(), x, y, yellow);
        }
    }
}
