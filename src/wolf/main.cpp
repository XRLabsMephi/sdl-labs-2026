#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"
#include <cstdio>
#include "world/Map.hpp"
#include <iostream>
#include <cassert>

int main(int argc, char* argv[]) {
    Map m;
    if (!m.load_from_file("C:\\Users\\Misha\\CLionProjects\\sdl-labs-2026\\assets\\wolf\\world\\map.txt")) {
        std::cerr << "load failed\n";
        return 1;
    }
    std::cout << m.get_width() << "x" << m.get_height() << "\n";

    std::cout << "wall(-1,5)="    << m.is_wall(-1, 5)    << " (expect 1)\n";
    std::cout << "wall(999,5)="   << m.is_wall(999, 5)   << " (expect 1)\n";
    std::cout << "wall(5,-1)="    << m.is_wall(5, -1)    << " (expect 1)\n";
    std::cout << "wall(5,999)="   << m.is_wall(5, 999)   << " (expect 1)\n";
    std::cout << "wall(0,0)="     << m.is_wall(0, 0)     << " (expect 1)\n";
    std::cout << "wall(3,3)="     << m.is_wall(3, 3)     << " (expect 0)\n";
    std::cout << "at(-1,5)="      << m.at(-1, 5)         << " (expect -1)\n";
    std::cout << "at(0,0)="       << m.at(0, 0)          << " (expect >0)\n";

    return 0;
}