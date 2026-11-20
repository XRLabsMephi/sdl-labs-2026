#include <algorithm>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <png.h>
#include <SDL3/SDL.h>

#include "wolf/effects/Fog.hpp"
#include "wolf/input/PlayerController.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/SdlPresenter.hpp"
#include "wolf/sprite/SpriteRenderer.hpp"
#include "support/Lab02Scene.hpp"

namespace {
    using Scene = lab02::DemoScene;

    void renderFrame(Scene& scene, raycast::Raycaster& walls,
                     const sprite::SpriteRenderer& sprites, const effects::Fog& fog,
                     render::Framebuffer& frame) {
        frame.clear({145, 138, 101});
        std::fill(frame.color() + frame.width() * (frame.height() / 2),
                  frame.color() + frame.width() * frame.height(), core::ColorRGBA(83, 77, 47).pack());
        walls.render(scene.player, scene.map, scene.textures, frame);
        sprites.render(scene.player, scene.sprites, scene.textures, walls.depthBuffer(), frame);
        fog.apply(frame);
    }

    void writePng(const render::Framebuffer& frame, const std::filesystem::path& path) {
        png_image image{};
        image.version = PNG_IMAGE_VERSION;
        image.width = frame.width();
        image.height = frame.height();
        image.format = PNG_FORMAT_RGBA;
        // Explicit channel extraction also works on big-endian hosts.
        std::vector<png_byte> bytes(static_cast<std::size_t>(frame.width()) * frame.height() * 4);
        for (std::size_t i = 0; i < bytes.size() / 4; ++i) {
            for (int channel = 0; channel < 4; ++channel) {
                bytes[i * 4 + channel] = static_cast<png_byte>(frame.color()[i] >> (8 * channel));
            }
        }
        if (!png_image_write_to_file(&image, path.string().c_str(), 0, bytes.data(), 0, nullptr)) {
            const std::string message = image.message;
            png_image_free(&image);
            throw std::runtime_error("Cannot write preview PNG: " + message);
        }
        png_image_free(&image);
    }

    input::InputState pollInput() {
        input::InputState state;
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
                state.set(input::InputState::kQuit);
            }
        }
        const auto* keys = SDL_GetKeyboardState(nullptr);
        if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) state.set(input::InputState::kForward);
        if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) state.set(input::InputState::kBackward);
        if (keys[SDL_SCANCODE_A]) state.set(input::InputState::kLeft);
        if (keys[SDL_SCANCODE_D]) state.set(input::InputState::kRight);
        if (keys[SDL_SCANCODE_LEFT]) state.set(input::InputState::kTurnLeft);
        if (keys[SDL_SCANCODE_RIGHT]) state.set(input::InputState::kTurnRight);
        return state;
    }
}

int main(int argc, char** argv) {
    try {
        const bool headless = argc == 3 && std::string(argv[1]) == "--output";
        if (argc != 1 && !headless) {
            std::cerr << "Usage: lab02_demo [--output preview.png]\n";
            return 1;
        }
        Scene scene;
        render::Framebuffer frame(960, 720);
        raycast::Raycaster walls;
        sprite::SpriteRenderer sprites;
        effects::Fog fog(3.0, 14.0, {160, 153, 107});
        if (headless) {
            renderFrame(scene, walls, sprites, fog, frame);
            writePng(frame, argv[2]);
            std::cout << "Preview: " << argv[2] << '\n';
            return 0;
        }

        render::SdlPresenter presenter(frame.width(), frame.height(), "LR2 - participant 3 demo");
        input::PlayerController controller(3.0, 2.5, 0.2);
        auto previous = std::chrono::steady_clock::now();
        while (true) {
            const auto input = pollInput();
            if (input.isSet(input::InputState::kQuit)) break;
            const auto now = std::chrono::steady_clock::now();
            const auto dt = std::chrono::duration<double>(now - previous).count();
            previous = now;
            controller.update(input, scene.player, scene.map, dt);
            renderFrame(scene, walls, sprites, fog, frame);
            presenter.present(frame);
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "LR2 demo failed: " << error.what() << '\n';
        return 1;
    }
}
