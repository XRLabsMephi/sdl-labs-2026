#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

constexpr int SCREEN_W = 1120;
constexpr int SCREEN_H = 640;

constexpr int MAP_W = 14;
constexpr int MAP_H = 11;

constexpr int RAYS = 140;
constexpr double PI = 3.14159265358979323846;

const std::vector<std::string> MAP = {
    "11111111111111",
    "10000000000001",
    "10000000000001",
    "10000000000001",
    "10000001000001",
    "10000001000001",
    "10000000000001",
    "10000000000001",
    "10000000000001",
    "10000000000001",
    "11111111111111"
};

struct Player
{
    double x = 3.0, y = 6.5;
    double angle = 0.0;
};

struct DDAStep
{
    int mapX = 0, mapY = 0;
    double sideDistX = 0.0, sideDistY = 0.0;
    int side = 0;
    bool wall = false;
};

struct RayTrace
{
    double cameraX = 0.0;
    double rayDirX = 0.0, rayDirY = 0.0;
    double hitX = 0.0, hitY = 0.0;
    double depth = 0.0;
    std::vector<DDAStep> steps;
};

bool isWall(int x, int y)
{
    if (x < 0 || y < 0 || x >= MAP_W || y >= MAP_H)
    {
        return true;
    }

    return MAP[y][x] != '0';
}

RayTrace buildTrace(const Player& player, int rayIndex, double fovDegrees)
{
    RayTrace trace;

    const double fov = fovDegrees * PI / 180.0;
    const double cameraDirX = std::cos(player.angle), cameraDirY = std::sin(player.angle);
    const double rightX = -cameraDirY, rightY = cameraDirX;
    const double plane = std::tan(fov / 2.0);

    trace.cameraX = 2.0 * ((rayIndex + 0.5) / static_cast<double>(RAYS)) - 1.0;
    trace.rayDirX = cameraDirX + rightX * plane * trace.cameraX;
    trace.rayDirY = cameraDirY + rightY * plane * trace.cameraX;

    int mapX = static_cast<int>(std::floor(player.x)), mapY = static_cast<int>(std::floor(player.y));

    const double deltaDistX = std::abs(1.0 / ((std::abs(trace.rayDirX) < 1e-9) ? 1e-9 : trace.rayDirX));
    const double deltaDistY = std::abs(1.0 / ((std::abs(trace.rayDirY) < 1e-9) ? 1e-9 : trace.rayDirY));

    int stepX, stepY;
    double sideDistX, sideDistY;

    if (trace.rayDirX < 0)
    {
        stepX = -1;
        sideDistX = (player.x - mapX) * deltaDistX;
    }
    else
    {
        stepX = 1;
        sideDistX = (mapX + 1.0 - player.x) * deltaDistX;
    }

    if (trace.rayDirY < 0)
    {
        stepY = -1;
        sideDistY = (player.y - mapY) * deltaDistY;
    }
    else
    {
        stepY = 1;
        sideDistY = (mapY + 1.0 - player.y) * deltaDistY;
    }

    int side = 0;

    for (int iteration = 0; iteration < 100; ++iteration)
    {
        DDAStep step;

        step.sideDistX = sideDistX;
        step.sideDistY = sideDistY;

        if (sideDistX < sideDistY)
        {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        }
        else
        {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }

        step.mapX = mapX;
        step.mapY = mapY;
        step.side = side;
        step.wall = isWall(mapX, mapY);

        trace.steps.push_back(step);

        if (step.wall)
        {
            break;
        }
    }

    if (side == 0)
    {
        trace.depth = (mapX - player.x + (1 - stepX) / 2.0) / trace.rayDirX;
    }
    else
    {
        trace.depth = (mapY - player.y + (1 - stepY) / 2.0) / trace.rayDirY;
    }

    trace.depth = std::max(0.0001, trace.depth);

    trace.hitX = player.x + trace.rayDirX * trace.depth;
    trace.hitY = player.y + trace.rayDirY * trace.depth;

    return trace;
}

void setColor(SDL_Renderer* renderer, Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255)
{
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

void fillRect(SDL_Renderer* renderer, float x, float y, float w, float h)
{
    SDL_FRect rect{x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void drawMap(SDL_Renderer* renderer, const Player& player, const RayTrace& trace, int visibleSteps)
{
    constexpr float CELL = 44.0f;
    constexpr float OFFSET_X = 40.0f;
    constexpr float OFFSET_Y = 90.0f;

    for (int y = 0; y < MAP_H; ++y)
    {
        for (int x = 0; x < MAP_W; ++x)
        {
            if (MAP[y][x] == '1')
            {
                setColor(renderer, 71, 85, 105);
            }
            else
            {
                setColor(renderer, 248, 250, 252);
            }

            fillRect(renderer, OFFSET_X + x * CELL, OFFSET_Y + y * CELL, CELL - 1, CELL - 1);
        }
    }

    const int count = std::min(visibleSteps, static_cast<int>(trace.steps.size()));

    for (int i = 0; i < count; ++i)
    {
        const DDAStep& step = trace.steps[i];

        if (i == count - 1)
        {
            setColor(renderer, 245, 158, 11, 150);
        }
        else
        {
            setColor(renderer, 245, 158, 11, 70);
        }

        fillRect(
            renderer,
            OFFSET_X + step.mapX * CELL + 4,
            OFFSET_Y + step.mapY * CELL + 4,
            CELL - 8,
            CELL - 8
        );
    }

    const float px = OFFSET_X + static_cast<float>(player.x * CELL);
    const float py = OFFSET_Y + static_cast<float>(player.y * CELL);

    float targetX = px;
    float targetY = py;

    if (count > 0)
    {
        if (count == static_cast<int>(trace.steps.size()) && trace.steps.back().wall)
        {
            targetX = OFFSET_X + static_cast<float>(trace.hitX * CELL);
            targetY = OFFSET_Y + static_cast<float>(trace.hitY * CELL);
        }
        else
        {
            const DDAStep& current = trace.steps[count - 1];

            targetX = OFFSET_X + (current.mapX + 0.5f) * CELL;
            targetY = OFFSET_Y + (current.mapY + 0.5f) * CELL;
        }
    }

    setColor(renderer, 220, 38, 38);
    SDL_RenderLine(renderer, px, py, targetX, targetY);

    setColor(renderer, 37, 99, 235);
    fillRect(renderer, px - 6, py - 6, 12, 12);

    setColor(renderer, 20, 30, 45);

    SDL_RenderLine(
        renderer,
        px,
        py,
        px + static_cast<float>(std::cos(player.angle) * CELL),
        py + static_cast<float>(std::sin(player.angle) * CELL)
    );
}

int main()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL error: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    if (!SDL_CreateWindowAndRenderer(
            "Raycaster - Step 2 DDA",
            SCREEN_W,
            SCREEN_H,
            0,
            &window,
            &renderer))
    {
        std::cerr << SDL_GetError() << '\n';

        SDL_Quit();
        return 1;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    Player player;

    double fov = 60.0;
    int selectedRay = 70;
    int ddaStep = 0;

    bool running = true;

    while (running)
    {
        RayTrace trace = buildTrace(player, selectedRay, fov);

        SDL_Event event;

        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN)
            {
                switch (event.key.key)
                {
                    case SDLK_ESCAPE:
                    {
                        running = false;
                        break;
                    }

                    case SDLK_RIGHT:
                    case SDLK_SPACE:
                    {
                        ddaStep = std::min(ddaStep + 1, static_cast<int>(trace.steps.size()));
                        break;
                    }

                    case SDLK_LEFT:
                    {
                        ddaStep = std::max(0, ddaStep - 1);
                        break;
                    }

                    case SDLK_UP:
                    {
                        selectedRay = std::min(RAYS - 1, selectedRay + 1);
                        ddaStep = 0;
                        break;
                    }

                    case SDLK_DOWN:
                    {
                        selectedRay = std::max(0, selectedRay - 1);
                        ddaStep = 0;
                        break;
                    }

                    case SDLK_R:
                    {
                        ddaStep = 0;
                        break;
                    }
                }
            }
        }

        trace = buildTrace(player, selectedRay, fov);

        ddaStep = std::clamp(ddaStep, 0, static_cast<int>(trace.steps.size()));

        std::ostringstream title;

        title << std::fixed
              << std::setprecision(3)
              << "Step 2 DDA"
              << " | Ray " << selectedRay
              << " | Step " << ddaStep << "/" << trace.steps.size();

        if (ddaStep > 0)
        {
            const DDAStep& step = trace.steps[ddaStep - 1];

            title << " | sideDistX=" << step.sideDistX
                  << " sideDistY=" << step.sideDistY
                  << " | cell=(" << step.mapX << "," << step.mapY << ")"
                  << " | ";

            if (step.sideDistX < step.sideDistY)
            {
                title << "X smaller -> X step";
            }
            else
            {
                title << "Y smaller -> Y step";
            }

            if (step.wall)
            {
                title << " | WALL -> STOP";
            }
            else
            {
                title << " | EMPTY";
            }
        }
        else
        {
            title << " | RIGHT/SPACE = next DDA step"
                  << " | UP/DOWN = ray";
        }

        SDL_SetWindowTitle(window, title.str().c_str());

        setColor(renderer, 244, 247, 251);
        SDL_RenderClear(renderer);

        drawMap(renderer, player, trace, ddaStep);

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}