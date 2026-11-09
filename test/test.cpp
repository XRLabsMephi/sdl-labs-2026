#include "wolf/render/FrameBuffer.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/world/Player.hpp"
#include "wolf/texture/TextureStore.hpp"
#include "wolf/input/PlayerController.hpp"
#include <SDL3/SDL.h>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include "wolf/sprite/Sprite.hpp"
#include "wolf/world/Map.hpp"
#include <fstream>
#include <cmath>


void testMovement() {
    const char* path = "test_movement_map.txt";
    {
        std::ofstream file(path);
        assert(file.is_open());
        file << "7 7\n";
        for (int y = 0; y < 7; y++) {
            for (int x = 0; x < 7; x++) {
                file << (x == 0 || y == 0 || x == 6 || y == 6 ? 1 : 0) << ' ';
            }
            file << '\n';
        }
    }
    Map map;
    assert(map.loadFromFile(path));
    std::remove(path);
    PlayerController controller;
    Player straight;
    straight.position = {3.5, 3.5};
    InputState input;
    input.forward = true;
    controller.update(straight, map, input, 0.05);
    double a = std::hypot(straight.position.x - 3.5, straight.position.y - 3.5);
    Player diagonal;
    diagonal.position = {3.5, 3.5};
    input.right = true;
    controller.update(diagonal, map, input, 0.05);
    double b = std::hypot(diagonal.position.x - 3.5, diagonal.position.y - 3.5);
    assert(std::abs(a - 0.15) < 1e-9);
    assert(std::abs(a - b) < 1e-9);
    Player stopped;
    stopped.position = {3.5, 3.5};
    input.forward = true;
    input.backward = true;
    input.right = false;
    controller.update(stopped, map, input, 0.05);
    assert(stopped.position.x == 3.5 && stopped.position.y == 3.5);
    std::cout << "Movement test passed\n";
}


void testRayHit() {
    const char* path = "test_map.txt";
    {
        std::ofstream file(path);
        file << "5 5\n";
        file << "1 1 1 1 1\n";
        file << "1 0 0 0 1\n";
        file << "1 0 0 0 1\n";
        file << "1 0 0 0 1\n";
        file << "1 1 1 1 1\n";
    }
    Map map;
    assert(map.loadFromFile(path));
    std::remove(path);
    Player player;
    player.position = {1.25, 1.75};
    player.direction = {1.0, 0.0};
    Raycaster raycaster;
    RayHit hit = raycaster.castRay(map, player, 0.0);
    assert(hit.hit);
    assert(hit.side == HitSide::X);
    assert(std::abs(hit.wallU - 0.75) < 1e-9);
    player.direction = {0.0, 1.0};
    hit = raycaster.castRay(map, player, 0.0);
    assert(hit.hit);
    assert(hit.side == HitSide::Y);
    assert(std::abs(hit.wallU - 0.25) < 1e-9);
    std::cout << "RayHit test passed\n";
}

void testTextureStore() {
    const char* path = "test_texture.bmp";
    SDL_Surface* surface = SDL_CreateSurface(2, 2, SDL_PIXELFORMAT_ARGB8888);
    assert(surface != nullptr);
    if (SDL_MUSTLOCK(surface)) {
        assert(SDL_LockSurface(surface));
    }
    auto* row0 = reinterpret_cast<Color*>(
        static_cast<unsigned char*>(surface->pixels)
    );
    auto* row1 = reinterpret_cast<Color*>(
        static_cast<unsigned char*>(surface->pixels) + surface->pitch
    );
    row0[0] = makeColor(255, 0, 0);
    row0[1] = makeColor(0, 255, 0);
    row1[0] = makeColor(0, 0, 255);
    row1[1] = makeColor(255, 255, 255);
    if (SDL_MUSTLOCK(surface)) {
        SDL_UnlockSurface(surface);
    }
    assert(SDL_SaveBMP(surface, path));
    SDL_DestroySurface(surface);
    TextureStore store;
    assert(store.load(5, path));
    const Texture* texture = store.get(5);
    assert(texture != nullptr);
    assert(texture->width == 2);
    assert(texture->height == 2);
    assert(texture->pixels.size() == 4);
    assert(texture->pixels[0] == makeColor(255, 0, 0));
    assert(texture->pixels[1] == makeColor(0, 255, 0));
    assert(texture->pixels[2] == makeColor(0, 0, 255));
    assert(texture->pixels[3] == makeColor(255, 255, 255));
    assert(store.get(100) == nullptr);
    assert(!store.load(6, "not_existing_file.bmp"));
    std::remove(path);
    std::cout << "TextureStore test passed\n";
}

void testSprite() {
    Sprite sprite{{4.5, 3.5}, 7, 0.75};
    assert(sprite.position.x == 4.5);
    assert(sprite.position.y == 3.5);
    assert(sprite.textureId == 7);
    assert(sprite.scale == 0.75);
    Sprite defaultSprite;
    assert(defaultSprite.textureId == 0);
    assert(defaultSprite.scale == 1.0);
    std::cout << "Sprite test passed\n";
}

int main() {
    FrameBuffer frame(3, 2);
    Color red = makeColor(255, 0, 0);
    Color green = makeColor(0, 255, 0);
    frame.clear(red);
    frame.setPixel(1, 1, green);
    frame.setPixel(-1, 0, green);
    frame.setPixel(3, 0, green);
    frame.setPixel(0, -1, green);
    frame.setPixel(0, 2, green);
    std::cout << frame.width() << ' ' << frame.height() << '\n';
    for (int y = 0; y < frame.height(); ++y) {
        for (int x = 0; x < frame.width(); ++x) {
            std::cout << std::hex << frame.data()[y * frame.width() + x] << ' ';
        }
        std::cout << '\n';
    }
    Player player;
    Raycaster raycaster;
    for (double cameraX : {-1.0, 0.0, 1.0}) {
        Vec2 direction = raycaster.rayDirection(player, cameraX);
        std::cout << direction.x << ' ' << direction.y << '\n';
    }
    try {
        FrameBuffer invalid(0, 2);
        std::cout << "ERROR: invalid size accepted\n";
        return 1;
    } catch (const std::invalid_argument&) {
        std::cout << "Invalid size rejected\n";
    }
    testTextureStore();
    testSprite();
    testRayHit();
    testMovement();
    return 0;
}