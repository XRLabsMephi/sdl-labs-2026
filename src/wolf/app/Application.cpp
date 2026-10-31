#include <app/Application.h>
#include <debug/Logger.h>

#include <exception>
#include <iostream>
#include <string>

namespace {

constexpr char WINDOW_TITLE[] = "RayCaster";

constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 768;

constexpr int FRAME_WIDTH = WINDOW_WIDTH;
constexpr int FRAME_HEIGHT = WINDOW_HEIGHT;

constexpr SDL_WindowFlags WINDOW_FLAGS = 0;

constexpr double NANOSECONDS_PER_SECOND = 1'000'000'000.0;

constexpr double MOVE_SPEED = 3.0;
constexpr double ROTATION_SPEED = 2.0;
constexpr double MOUSE_SENSITIVITY = 0.002;

enum class ApplicationError {
    MapLoading = 1,
    InvalidSpawn,
    SdlInitialization,
    WindowCreation,
    PresenterInitialization,
    InitializationException,
    NotInitialized,
    FramePresentation,
    RuntimeException
};

// Preserve the original error even if writing to the log file fails.
void reportError(
    ApplicationError code,
    const std::string& message,
    const std::string& context
) noexcept {
    try {
        logging::error(
            "APP",
            static_cast<int>(code),
            message,
            context
        );
    } catch (...) {
        try {
            std::cerr << message
                      << " | Context: " << context
                      << '\n';
        } catch (...) {
        }
    }
}

}

app::Application::Application() = default;

app::Application::~Application() {
    shutdown();
}

bool app::Application::initialize(const std::string& mapFilename) {
    if (initialized) {
        return true;
    }

    try {
        logging::info("APP", "Initializing application.");

        if (!map.loadFromFile(mapFilename)) {
            reportError(
                ApplicationError::MapLoading,
                "Failed to load map.",
                "Application::initialize File=\"" + mapFilename + "\""
            );

            return false;
        }

        if (!map.canOccupy(
                map.getSpawnPosition(),
                player.getRadius()
            )) {
            reportError(
                ApplicationError::InvalidSpawn,
                "The player cannot occupy the spawn position.",
                "Application::initialize"
            );

            return false;
        }

        player.setPosition(map.getSpawnPosition());

        // Initialize all controller settings before the first update.
        playerController.setMoveSpeed(MOVE_SPEED);
        playerController.setRotationSpeed(ROTATION_SPEED);
        playerController.setMouseSensitivity(MOUSE_SENSITIVITY);

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            reportError(
                ApplicationError::SdlInitialization,
                SDL_GetError(),
                "Application::initialize SDL_Init"
            );

            return false;
        }

        sdlInitialized = true;

        window.reset(
            SDL_CreateWindow(
                WINDOW_TITLE,
                WINDOW_WIDTH,
                WINDOW_HEIGHT,
                WINDOW_FLAGS
            )
        );

        if (!window) {
            reportError(
                ApplicationError::WindowCreation,
                SDL_GetError(),
                "Application::initialize SDL_CreateWindow"
            );

            shutdown();
            return false;
        }

        if (!SDL_SetWindowRelativeMouseMode(window.get(), true)) {
            logging::warning(
                "APP",
                std::string("Failed to enable relative mouse mode: ")
                + SDL_GetError()
            );
        }

        frameBuffer = std::make_unique<render::FrameBuffer>(
            FRAME_WIDTH,
            FRAME_HEIGHT
        );

        // RayCaster stores references to the player, map and frame buffer.
        rayCaster = std::make_unique<raycast::RayCaster>(
            FRAME_WIDTH,
            FRAME_HEIGHT,
            player,
            map,
            *frameBuffer
        );

        // The presenter borrows the window; Application retains ownership.
        presenter = std::make_unique<render::SdlPresenter>(
            window.get()
        );

        const std::string presenterError = presenter->getError();

        if (!presenterError.empty()) {
            reportError(
                ApplicationError::PresenterInitialization,
                presenterError,
                "Application::initialize SdlPresenter"
            );

            shutdown();
            return false;
        }

        input.reset();

        focused =
            (SDL_GetWindowFlags(window.get()) & SDL_WINDOW_INPUT_FOCUS)
            != 0;

        logging::info(
            "APP",
            "Application initialized successfully."
        );

        initialized = true;

        return true;
    } catch (const std::exception& error) {
        reportError(
            ApplicationError::InitializationException,
            error.what(),
            "Application::initialize"
        );

        shutdown();
        return false;
    } catch (...) {
        reportError(
            ApplicationError::InitializationException,
            "Unknown initialization error.",
            "Application::initialize"
        );

        shutdown();
        return false;
    }
}

void app::Application::run() {
    if (!initialized) {
        reportError(
            ApplicationError::NotInitialized,
            "Application must be initialized before running.",
            "Application::run"
        );

        return;
    }

    if (running) {
        return;
    }

    running = true;

    try {
        logging::info("APP", "Starting the main loop.");

        Uint64 previousTime = SDL_GetTicksNS();

        while (running) {
            const Uint64 currentTime = SDL_GetTicksNS();

            // Convert elapsed time from nanoseconds to seconds.
            const double deltaTime =
                static_cast<double>(currentTime - previousTime)
                / NANOSECONDS_PER_SECOND;

            previousTime = currentTime;

            input.beginFrame();
            processEvents();

            if (!running) {
                break;
            }

            update(deltaTime);

            // Clear the previous frame before drawing the current scene.
            frameBuffer->clear(render::DrawCallParams{});

            rayCaster->DoRayCasting();

            if (!presenter->present(*frameBuffer)) {
                reportError(
                    ApplicationError::FramePresentation,
                    presenter->getError(),
                    "Application::run SdlPresenter::present"
                );

                stop();
            }
        }

        logging::info("APP", "The main loop has stopped.");
    } catch (const std::exception& error) {
        reportError(
            ApplicationError::RuntimeException,
            error.what(),
            "Application::run"
        );
    } catch (...) {
        reportError(
            ApplicationError::RuntimeException,
            "Unknown runtime error.",
            "Application::run"
        );
    }

    shutdown();
}

void app::Application::processEvents() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                stop();
                return;

            case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                if (event.window.windowID ==
                    SDL_GetWindowID(window.get())) {
                    stop();
                    return;
                }
                break;

            case SDL_EVENT_KEY_DOWN:
                if (focused && event.key.key == SDLK_ESCAPE) {
                    stop();
                    return;
                }

                if (focused) {
                    input.handleEvent(event);
                }
                break;

            case SDL_EVENT_WINDOW_FOCUS_LOST:
                if (event.window.windowID ==
                    SDL_GetWindowID(window.get())) {
                    focused = false;
                    input.reset();
                }
                break;

            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                if (event.window.windowID ==
                    SDL_GetWindowID(window.get())) {
                    focused = true;
                    input.reset();
                }
                break;

            default:
                if (focused) {
                    input.handleEvent(event);
                }
                break;
        }
    }
}

void app::Application::update(double deltaTime) {
    if (!focused) {
        return;
    }

    playerController.update(
        player,
        map,
        input,
        deltaTime
    );
}

void app::Application::shutdown() noexcept {
    running = false;
    initialized = false;
    focused = false;

    input.reset();

    // Release renderer resources before destroying the borrowed window.
    presenter.reset();

    // Destroy objects holding references before their referenced resources.
    rayCaster.reset();
    frameBuffer.reset();

    window.reset();

    if (sdlInitialized) {
        SDL_Quit();
        sdlInitialized = false;
    }
}