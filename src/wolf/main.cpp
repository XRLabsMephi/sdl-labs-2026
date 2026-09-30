#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include <cstdio>
#include "world/Map.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    Map map;
    bool success = map.load_from_file("");
    std::cout << success;
    return 0;
}