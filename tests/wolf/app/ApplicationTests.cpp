#include "wolf/app/Application.hpp"
#include "wolf/world/SceneResources.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <atomic>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string_view>

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
    const wolf::Application* application = nullptr;
    bool sceneReadyBeforeWindow = false;
    const wolf::Color* texturePixels = nullptr;
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
        const auto& scene = request.application->scene();
        request.sceneReadyBeforeWindow = scene.textures.size() == 6 && scene.sprites.size() == 4;
        for (const auto& resource : wolf::defaultSceneResources())
        {
            request.sceneReadyBeforeWindow &= scene.textures.contains(resource.textureId);
        }
        if (scene.textures.contains(wolf::sceneTexture::brick))
        {
            request.texturePixels = scene.textures.get(wolf::sceneTexture::brick).pixels.data();
        }
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
    wolf::Application app;
    request.application = &app;
    require(SDL_Init(SDL_INIT_EVENTS), "Could not initialize SDL test events");
    require(SDL_AddEventWatch(watchWindow, &request), "Could not observe the SDL window");
    require(app.run() == 0, "Application must exit successfully");
    require(request.sceneReadyBeforeWindow,
            "Application must prepare textures and actual Sprite objects before creating its window");
    const auto& scene = app.scene();
    require(scene.textures.size() == 6 && scene.sprites.size() == 4 && request.texturePixels &&
                scene.textures.get(wolf::sceneTexture::brick).pixels.data() == request.texturePixels,
            "Prepared CPU data must remain available through the loop and after SDL cleanup");
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

void testResourceFailure()
{
    const auto directory = wolf::defaultResourceDirectory();
    require(!std::filesystem::exists(directory), "The isolated application must not have resources");
    wolf::Application app;
    std::ostringstream errors;
    auto* previous = std::cerr.rdbuf(errors.rdbuf());
    const int result = app.run();
    std::cerr.rdbuf(previous);
    require(result == 1 && SDL_WasInit(0) == 0,
            "Resource preparation failure must return an error and quit SDL");
    require(errors.str().find(directory.string()) != std::string::npos,
            "Application must report the failed resource path");
    require(app.scene().textures.size() == 0 && app.scene().sprites.empty(),
            "Resource failure must not publish a partially prepared scene");
}

}

int main(int argc, char* argv[])
{
    try
    {
        if (argc == 2 && std::string_view(argv[1]) == "--missing-resources")
        {
            testResourceFailure();
            std::cout << "Application resource failure test passed\n";
            return 0;
        }
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
