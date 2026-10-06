#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>
#include "playfield.h"

/*
    Функция вычисления 1 луча. 
    Возвращает расстояние до обьекта, с которым произошло пересечение. 
    -1 если пересечения в пределах 1.000 не произошло
*/
double RayTracing(double startPosX, double startPosY, double angle, Playfield& pf) {
    for (double distance = 0; distance < 1000; distance += 0.5) {
        double x = startPosX + distance * std::cos(angle);
        double y = startPosY + distance * std::sin(angle);

        for (auto& board : pf.boards) {
            if (board.CheckConflict(x, y)) {
                return distance;
            }
        }
    }

    return -1;
}

int main(int argc, char* argv[]) {
    //работа с игровым полем
    int k = 10;
    Playfield play_field(10, 10, k);

    play_field.set_rectangle(7, 6, 9, 8, 1);

    play_field.print();
    

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
    
    //стартовая точка & угол поворота камеры
    double PlayerCoordX = k + 1;
    double PlayerCoordY = k + 1;
    double PlayerA = std::numbers::pi / 4;

    //размеры экрана
    const int screenWidth = 800;
    const int screenHeight = 600;

    //кол-во лучей на экран
    const int raysCount = 800;

    //угол обзора
    const double FOV = std::numbers::pi / 2;

    /*
        расстояние до плоскости экрана, на который проэцируется все обьекты из точки игрока
    */
    const double projectionDistance =
        (screenWidth / 2.0) / std::tan(FOV / 2.0);

    bool running = true;

    //основной цикл для отрисовки
    while (running) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        const bool* keys = SDL_GetKeyboardState(nullptr);

        //перемещение
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

        //закрашиваем весь экран в темно-серый
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
        SDL_RenderClear(renderer);

        //для каждого луча при надобности отрисовываем столкновение со стенкой
        for (int i = 0; i < raysCount; i++) {
            double rayAngle =
                PlayerA - FOV / 2 +
                FOV * i / raysCount;

            double distance =
                RayTracing(PlayerCoordX, PlayerCoordY, rayAngle, play_field);

            if (distance < 0) {
                continue;
            }

            //тут фиксится рыбий глаз
            double correctedDistance =
                distance * std::cos(rayAngle - PlayerA);
            //за что пофиксили - не ясно, ибо просили пофиксить на этапе сильно позже :/


            double height =
                k *
                projectionDistance /
                correctedDistance;

            SDL_FRect rect{
                static_cast<float>(i),
                static_cast<float>(screenHeight / 2.0 - height / 2.0),
                1.0f,
                static_cast<float>(height)
            };
            
            //коэффициент затухания цвета в зависимости от дальности
            double brightness = 1.0 - correctedDistance / 300.0;
            if (brightness < 0.0) brightness = 0.0;
            if (brightness > 1.0) brightness = 1.0;

            //повышаем контрастность
            brightness = std::pow(brightness, 4);
            
            SDL_SetRenderDrawColor(renderer, 0 * brightness, 150 * brightness,
                255 * brightness, 255 * brightness);
            SDL_RenderFillRect(renderer, &rect);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}