
#include "wolf/core/Color.hpp"
#include "wolf/effects/Fog.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/sprite/Sprite.hpp"
#include "wolf/sprite/SpriteRenderer.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/world/Map.hpp"
#include <SDL3/SDL.h>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

void require(bool ok, const char* message) {
    if (!ok) throw std::runtime_error(message);
}

int main() {
    const char* mapPath = "render_check_map.txt";
    const char* spritePath = "render_check_sprite.bmp";
    try {
        {
            std::ofstream file(mapPath);
            require(file.is_open(), "Cannot create test map");
            file << "5 5\n";
            for (int y = 0; y < 5; y++) {
                for (int x = 0; x < 5; x++) {
                    file << (x == 0 || y == 0 || x == 4 || y == 4 ? 1 : 0) << ' ';
                }
                file << '\n';
            }
        }
        Map map;
        require(map.loadFromFile(mapPath), "Map loading failed");
        Player player;
        player.position = {1.5, 1.5};
        Raycaster raycaster;
        FrameBuffer frame(80, 60);
        TextureStore emptyTextures;
        Fog noFog{100.0, 200.0, makeColor(0, 0, 0)};
        raycaster.render(map, player, frame, emptyTextures, noFog);
        require(raycaster.depthBuffer().size() == 80, "Wrong depth buffer size");
        require(std::abs(raycaster.depthBuffer()[40] - 2.5) < 1e-9, "Wrong wall depth");
        require(frame.data()[30 * 80 + 40] == makeColor(200, 70, 70), "Wall rendering failed");
        std::cout << "Wall rendering passed\n";
        SDL_Surface* surface = SDL_CreateSurface(2, 2, SDL_PIXELFORMAT_ARGB8888);
        require(surface != nullptr, "Cannot create sprite surface");
        bool locked = SDL_MUSTLOCK(surface);
        if (locked && !SDL_LockSurface(surface)) {
            SDL_DestroySurface(surface);
            throw std::runtime_error("Cannot lock surface");
        }
        for (int y = 0; y < 2; y++) {
            auto* row = reinterpret_cast<Color*>(
                static_cast<unsigned char*>(surface->pixels) + y * surface->pitch
            );
            for (int x = 0; x < 2; x++) {
                row[x] = makeColor(0, 255, 0);
            }
        }
        if (locked) SDL_UnlockSurface(surface);
        bool saved = SDL_SaveBMP(surface, spritePath);
        SDL_DestroySurface(surface);
        require(saved, "Cannot save test sprite");
        TextureStore textures;
        require(textures.load(5, spritePath), "Cannot load test sprite");
        SpriteRenderer spriteRenderer;
        std::vector<Sprite> front = {{{2.5, 1.5}, 5, 0.5}};
        spriteRenderer.render(front, player, frame, textures, raycaster.depthBuffer(), noFog);
        require(frame.data()[45 * 80 + 40] == makeColor(0, 255, 0), "Visible sprite not drawn");
        std::cout << "Visible sprite passed\n";
        raycaster.render(map, player, frame, emptyTextures, noFog);
        std::vector<Sprite> behind = {{{4.5, 1.5}, 5, 1.0}};
        spriteRenderer.render(behind, player, frame, textures, raycaster.depthBuffer(), noFog);
        require(frame.data()[30 * 80 + 40] == makeColor(200, 70, 70), "Sprite drawn through wall");
        std::cout << "Wall occlusion passed\n";
        Fog fog{0.0, 5.0, makeColor(0, 0, 0)};
        raycaster.render(map, player, frame, emptyTextures, fog);
        require(frame.data()[30 * 80 + 40] == makeColor(100, 35, 35), "Fog rendering failed");
        std::cout << "Fog rendering passed\n";
        std::remove(mapPath);
        std::remove(spritePath);
        std::cout << "Render check passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::remove(mapPath);
        std::remove(spritePath);
        std::cerr << error.what() << '\n';
        return 1;
    }
}
