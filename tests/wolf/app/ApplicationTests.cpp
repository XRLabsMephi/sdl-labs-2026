#include "wolf/app/Application.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <iostream>
#include <memory>
#include <stdexcept>

namespace
{

void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

struct RenderResources
{
    SDL_WindowID windowId;
    bool& releasedBeforeWindow;

    ~RenderResources()
    {
        releasedBeforeWindow = SDL_GetWindowFromID(windowId) != nullptr &&
                               SDL_WasInit(SDL_INIT_VIDEO) != 0;
    }
};

enum class ExitEvent { Quit, WindowClose, Escape };

void testExit(ExitEvent exitEvent)
{
    int frames = 0;
    bool releasedBeforeWindow = false;
    SDL_WindowID windowId = 0;
    std::weak_ptr<RenderResources> resources;
    wolf::Application app([&](SDL_Window& window) -> wolf::Application::RenderFrame
    {
        windowId = SDL_GetWindowID(&window);
        auto owned = std::make_shared<RenderResources>(windowId, releasedBeforeWindow);
        resources = owned;
        return [&, owned](const wolf::Map& map, const wolf::Player& player)
        {
            require(map.isEmpty(1, 1) && player.position.x == 1.5,
                    "Application must provide the initial scene to the renderer");
            ++frames;
            require(frames <= 3, "Exit event did not stop the main loop");
            SDL_Event event{};
            if (frames == 1)
            {
                event.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
                event.window.windowID = windowId + 100;
            }
            else if (exitEvent == ExitEvent::Quit)
            {
                event.type = SDL_EVENT_QUIT;
            }
            else if (exitEvent == ExitEvent::WindowClose)
            {
                event.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
                event.window.windowID = windowId;
            }
            else
            {
                event.type = SDL_EVENT_KEY_DOWN;
                event.key.windowID = windowId;
                event.key.scancode = SDL_SCANCODE_ESCAPE;
                event.key.down = true;
            }
            require(SDL_PushEvent(&event), "Could not queue an SDL exit event");
        };
    });
    require(app.run() == 0 && frames == 2, "Application must ignore other windows and then exit");
    require(resources.expired() && releasedBeforeWindow,
            "Rendering resources must be released before the window and SDL");
    require(SDL_WasInit(0) == 0 && SDL_GetWindowFromID(windowId) == nullptr,
            "Application must release the SDL window and subsystems");
    require(app.map().width() == 12 && app.player().direction.x == 1.0,
            "Scene state must remain accessible after shutdown");
}

void testDiagnostics()
{
    wolf::Application app([](SDL_Window&) -> wolf::Application::RenderFrame
    {
        const auto timer = SDL_AddTimer(100,
            [](void*, SDL_TimerID, Uint32) -> Uint32
            {
                SDL_Event event{};
                event.type = SDL_EVENT_QUIT;
                SDL_PushEvent(&event);
                return 0;
            }, nullptr);
        require(timer != 0, "Could not schedule diagnostic window shutdown");
        return {};
    });
    require(app.run() == 0, "Diagnostic window must run without rendering modules");
    require(SDL_WasInit(0) == 0, "Diagnostic mode must clean up SDL");
}

void testFailures()
{
    SDL_WindowID windowId = 0;
    wolf::Application factoryFailure([&](SDL_Window& window) -> wolf::Application::RenderFrame
    {
        windowId = SDL_GetWindowID(&window);
        throw std::runtime_error("Expected renderer initialization failure");
    });
    require(factoryFailure.run() == 1 && SDL_WasInit(0) == 0 &&
                SDL_GetWindowFromID(windowId) == nullptr,
            "Renderer initialization failure must clean up the window and SDL");

    bool releasedBeforeWindow = false;
    wolf::Application frameFailure([&](SDL_Window& window) -> wolf::Application::RenderFrame
    {
        auto owned = std::make_shared<RenderResources>(SDL_GetWindowID(&window),
                                                      releasedBeforeWindow);
        return [owned](const wolf::Map&, const wolf::Player&)
        {
            throw std::runtime_error("Expected frame failure");
        };
    });
    require(frameFailure.run() == 1 && releasedBeforeWindow && SDL_WasInit(0) == 0,
            "A frame failure must release resources in the correct order");

    require(SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "wolf-invalid-driver", SDL_HINT_OVERRIDE),
            "Could not configure the initialization failure test");
    wolf::Application initFailure;
    require(initFailure.run() == 1 && SDL_WasInit(0) == 0,
            "SDL initialization failure must return an error and clean up");
    SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
}

}

int main(int, char*[])
{
    try
    {
        testExit(ExitEvent::Quit);
        testExit(ExitEvent::WindowClose);
        testExit(ExitEvent::Escape);
        testDiagnostics();
        testFailures();
        // Also verifies that SDL can be initialized again after a failed run.
        testExit(ExitEvent::Quit);
        std::cout << "All Application tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
