#include "wolf/app/Application.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <atomic>
#include <iostream>
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

struct ExitRequest
{
    Uint32 eventType;
    SDL_WindowID windowId = 0;
    SDL_TimerID timer = 0;
    int timerTicks = 0;
    std::atomic_bool otherCloseQueued = false;
    std::atomic_bool exitQueued = false;
    bool diagnosticsShown = false;
    bool destroyedBeforeQuit = false;
    bool presenterDestroyed = false;
};

Uint32 SDLCALL sendExit(void* userdata, SDL_TimerID, Uint32)
{
    auto& request = *static_cast<ExitRequest*>(userdata);
    SDL_Event event{};
    if (request.timerTicks++ == 0)
    {
        event.type = SDL_EVENT_WINDOW_CLOSE_REQUESTED;
        event.window.windowID = request.windowId + 100;
        request.otherCloseQueued = SDL_PushEvent(&event);
        return 50;
    }

    event.type = request.eventType;
    if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
    {
        event.window.windowID = request.windowId;
    }
    else if (event.type == SDL_EVENT_KEY_DOWN)
    {
        event.key.windowID = request.windowId;
        event.key.scancode = SDL_SCANCODE_ESCAPE;
        event.key.down = true;
    }
    request.exitQueued = SDL_PushEvent(&event);
    return 0;
}

bool SDLCALL watchWindow(void* userdata, SDL_Event* event)
{
    auto& request = *static_cast<ExitRequest*>(userdata);
    if (event->type == SDL_EVENT_WINDOW_SHOWN && request.windowId == 0)
    {
        request.windowId = event->window.windowID;
        request.timer = SDL_AddTimer(50, sendExit, &request);
    }
    else if (event->type == SDL_EVENT_WINDOW_DESTROYED &&
             event->window.windowID == request.windowId)
    {
        SDL_Window* window = SDL_GetWindowFromID(request.windowId);
        request.destroyedBeforeQuit = window && SDL_WasInit(SDL_INIT_VIDEO) != 0;
        request.presenterDestroyed = window && SDL_GetRenderer(window) == nullptr;
        request.diagnosticsShown = window &&
            SDL_strstr(SDL_GetWindowTitle(window), "Wolf Lab 1 | pos 1.50, 1.50") &&
            SDL_strstr(SDL_GetWindowTitle(window), "dir 1.00, 0.00");
    }
    return true;
}

void testExit(Uint32 eventType)
{
    ExitRequest request{eventType};
    require(SDL_Init(SDL_INIT_EVENTS), "Could not initialize SDL test events");
    require(SDL_AddEventWatch(watchWindow, &request), "Could not observe the SDL window");
    wolf::Application app;
    require(app.run() == 0, "Application must exit successfully");
    require(request.windowId != 0 && request.timer != 0 && request.otherCloseQueued &&
                request.exitQueued, "Application must ignore other windows before handling exit");
    require(request.diagnosticsShown, "The main loop must display initial camera diagnostics");
    require(request.presenterDestroyed && request.destroyedBeforeQuit && SDL_WasInit(0) == 0 &&
                SDL_GetWindowFromID(request.windowId) == nullptr,
            "Application must destroy its window before quitting SDL");
}

void testInitializationFailure()
{
    require(SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "wolf-invalid-driver", SDL_HINT_OVERRIDE),
            "Could not configure the initialization failure test");
    wolf::Application app;
    const int result = app.run();
    SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
    require(result == 1 && SDL_WasInit(0) == 0,
            "SDL initialization failure must return an error and clean up");
}

void testPresenterFailure()
{
    require(SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "wolf-invalid-renderer", SDL_HINT_OVERRIDE),
            "Could not configure the presenter failure test");
    wolf::Application app;
    const int result = app.run();
    SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
    require(result == 1 && SDL_WasInit(0) == 0,
            "Presenter initialization failure must return an error and clean up SDL");
}

}

int main(int, char*[])
{
    try
    {
        testExit(SDL_EVENT_QUIT);
        testExit(SDL_EVENT_WINDOW_CLOSE_REQUESTED);
        testExit(SDL_EVENT_KEY_DOWN);
        testInitializationFailure();
        testPresenterFailure();
        testExit(SDL_EVENT_QUIT);
        std::cout << "All Application tests passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        SDL_Quit();
        std::cerr << error.what() << '\n';
        return 1;
    }
}
