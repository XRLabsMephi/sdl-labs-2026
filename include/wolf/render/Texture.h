#pragma once
#include <string>
#include <vector>
#include <SDL3/SDL_stdinc.h>

struct Texture
{
    int width = 0;
    int height = 0;

    std::vector<Uint32> pixels;

    Uint32 pixel(int x, int y) const;

    explicit Texture(const std::string& filename);

    static Texture createFromFile(const std::string& filename);
};
