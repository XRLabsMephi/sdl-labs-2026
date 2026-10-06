#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>

class Board {
public:
    double LenghtX;
    double LenghtY;
    double LenghtZ;
    double CoordX;
    double CoordY;
    double CoordZ;

    Board(double CoordX, double CoordY, double CoordZ,
          double LenghtX, double LenghtY, double LenghtZ)
        : CoordX(CoordX), CoordY(CoordY), CoordZ(CoordZ),
          LenghtX(LenghtX), LenghtY(LenghtY), LenghtZ(LenghtZ) {
    }

    bool CheckConflict(double x, double y) {
        return x >= CoordX &&
               x <= CoordX + LenghtX &&
               y >= CoordY &&
               y <= CoordY + LenghtY;
    }
};

std::vector<Board> Boards;

double RayTracing(double startPosX, double startPosY, double angle) {
    for (double distance = 0; distance < 1000; distance += 0.5) {
        double x = startPosX + distance * std::cos(angle);
        double y = startPosY + distance * std::sin(angle);

        for (auto& board : Boards) {
            if (board.CheckConflict(x, y)) {
                return distance;
            }
        }
    }

    return -1;
}

int main(int argc, char* argv[]) {
    Boards.push_back(Board(400, 400, 0, 50, 50, 50));

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_CreateWindowAndRenderer(
        "Raycasting",
        800,
        600,
        0,
        &window,
        &renderer
    )) {
        std::cout << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    double PlayerCoordX = 0;
    double PlayerCoordY = 0;
    double PlayerA = std::numbers::pi / 4;

    const int screenWidth = 800;
    const int screenHeight = 600;

    const int raysCount = 800;

    const double FOV = std::numbers::pi / 2;


    const double projectionDistance =
        (screenWidth / 2.0) / std::tan(FOV / 2.0);

    bool running = true;

    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        const bool* keys = SDL_GetKeyboardState(nullptr);

        if (keys[SDL_SCANCODE_A]) {
            PlayerA -= 0.02;
        }

        if (keys[SDL_SCANCODE_D]) {
            PlayerA += 0.02;
        }

        if (keys[SDL_SCANCODE_W]) {
            PlayerCoordX += std::cos(PlayerA) * 2;
            PlayerCoordY += std::sin(PlayerA) * 2;
        }

        if (keys[SDL_SCANCODE_S]) {
            PlayerCoordX -= std::cos(PlayerA) * 2;
            PlayerCoordY -= std::sin(PlayerA) * 2;
        }

        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < raysCount; i++) {
            double rayAngle =
                PlayerA - FOV / 2 +
                FOV * i / raysCount;

            double distance =
                RayTracing(PlayerCoordX, PlayerCoordY, rayAngle);

            if (distance < 0) {
                continue;
            }

            double correctedDistance =
                distance * std::cos(rayAngle - PlayerA);

            double height =
                Boards[0].LenghtZ *
                projectionDistance /
                correctedDistance;

            SDL_FRect rect{
                static_cast<float>(i),
                static_cast<float>(screenHeight / 2.0 - height / 2.0),
                1.0f,
                static_cast<float>(height)
            };

            SDL_SetRenderDrawColor(renderer, 0, 150, 255, 255);
            SDL_RenderFillRect(renderer, &rect);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}