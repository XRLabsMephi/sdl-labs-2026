#include <app/Application.h>
#include <debug/Logger.h>
#include <render/TextureStorageLoader.h>

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <numbers>

namespace {

constexpr char WINDOW_TITLE[] = "RayCaster";

constexpr int WINDOW_WIDTH = 1024;
constexpr int WINDOW_HEIGHT = 768;

constexpr int EYE_HEIGHT = WINDOW_HEIGHT / 2;

constexpr int FRAME_WIDTH = WINDOW_WIDTH;
constexpr int FRAME_HEIGHT = WINDOW_HEIGHT;
constexpr core::Color FOG_COLOR{72, 76, 84};
constexpr float MIN_FOG_DEPTH = 2.0f;
constexpr float MAX_FOG_DEPTH = 12.0f;

constexpr SDL_WindowFlags WINDOW_FLAGS = SDL_WINDOW_RESIZABLE;


constexpr double NANOSECONDS_PER_SECOND = 1e-9;

constexpr double MOVE_SPEED = 3.0;
constexpr double ROTATION_SPEED = 2.0;
constexpr double MOUSE_SENSITIVITY = 0.002;

constexpr double FIELD_OF_VIEW = std::numbers::pi / 3;

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
    if (initialized_) {
        return true;
    }

    try {
        logging::info("APP", "Initializing application.");

        namespace fs = std::filesystem;
        const char* base = SDL_GetBasePath();
        const fs::path root = base ? fs::path(base) : fs::current_path();
        const fs::path mapPath = root / mapFilename;

        if (!map_.loadFromFile(mapPath.string())) {
            reportError(
                ApplicationError::MapLoading,
                "Failed to load map.",
                "Application::initialize File=\"" + mapPath.string() + "\""
            );

            return false;
        }

        if (!map_.canOccupy(
                map_.getSpawnPosition(),
                player_.getRadius()
            )) {
            reportError(
                ApplicationError::InvalidSpawn,
                "The player cannot occupy the spawn position.",
                "Application::initialize"
            );

            return false;
        }

        player_.setPosition(map_.getSpawnPosition());
        player_.setPosition(map_.getSpawnPosition());
        player_.setEyeHeigth(EYE_HEIGHT);
        player_.setAngle(FIELD_OF_VIEW);

        // Initialize all controller settings before the first update.
        playerController_.setMoveSpeed(MOVE_SPEED);
        playerController_.setRotationSpeed(ROTATION_SPEED);
        playerController_.setMouseSensitivity(MOUSE_SENSITIVITY);

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            reportError(
                ApplicationError::SdlInitialization,
                SDL_GetError(),
                "Application::initialize SDL_Init"
            );

            return false;
        }

        sdlInitialized_ = true;

        window_.reset(
            SDL_CreateWindow(
                WINDOW_TITLE,
                WINDOW_WIDTH,
                WINDOW_HEIGHT,
                WINDOW_FLAGS
            )
        );

        if (!window_) {
            reportError(
                ApplicationError::WindowCreation,
                SDL_GetError(),
                "Application::initialize SDL_CreateWindow"
            );

            shutdown();
            return false;
        }

        if (!SDL_SetWindowRelativeMouseMode(window_.get(), true)) {
            logging::warning(
                "APP",
                std::string("Failed to enable relative mouse mode: ")
                + SDL_GetError()
            );
        }

        render::TextureStorageLoader(
            root / "assets\\текстуры\\textures_mapping.txt"
            ).load(textureStorage_);

        frameBuffer_ = std::make_unique<render::FrameBuffer>(
            FRAME_WIDTH,
            FRAME_HEIGHT
        );

        frameBuffer_->set_fog_params(FOG_COLOR, MIN_FOG_DEPTH, MAX_FOG_DEPTH);

        viewSettings_ = ViewSettings{};
        // RayCaster borrows the player, map, frame buffer and textures.
        rayCaster_ = std::make_unique<raycast::RayCaster>(
            player_,
            map_,
            *frameBuffer_,
            textureStorage_,
            viewSettings_
        );

        rayCaster_->set_fog_params(FOG_COLOR, MIN_FOG_DEPTH, MAX_FOG_DEPTH);

        spriteRenderer_ = std::make_unique<render::SpriteRenderer>(
            player_,
            map_,
            *frameBuffer_,
            textureStorage_,
            viewSettings_,
            rayCaster_->getDepthBuffer()
        );

        // The presenter borrows the window; Application retains ownership.
        presenter_ = std::make_unique<render::SdlPresenter>(
            window_.get()
        );

        const std::string presenterError = presenter_->getError();

        if (!presenterError.empty()) {
            reportError(
                ApplicationError::PresenterInitialization,
                presenterError,
                "Application::initialize SdlPresenter"
            );

            shutdown();
            return false;
        }

        input_.reset();

        focused_ =
                (SDL_GetWindowFlags(window_.get()) & SDL_WINDOW_INPUT_FOCUS)
                != 0;

        logging::info(
            "APP",
            "Application initialized successfully."
        );

        initialized_ = true;

        return true;
    } catch (const std::exception &error) {
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
    if (!initialized_) {
        reportError(
            ApplicationError::NotInitialized,
            "Application must be initialized before running.",
            "Application::run"
        );

        return;
    }

    if (running_) {
        return;
    }

    running_ = true;

    try {
        logging::info("APP", "Starting the main loop.");

        Uint64 previousTime = SDL_GetTicksNS();
        Uint64 fpsStartTime = previousTime;
        Uint64 fpsFrameCount = 0;
        while (running_) {
            const Uint64 currentTime = SDL_GetTicksNS();

            // Convert elapsed time from nanoseconds to seconds.
            const double deltaTime =
                    static_cast<double>(currentTime - previousTime)
                    * NANOSECONDS_PER_SECOND;

            previousTime = currentTime;

            input_.beginFrame();
            processEvents();

            if (!running_) {
                break;
            }

            update(deltaTime);

            rayCaster_->renderScene();
            spriteRenderer_->renderSprites();

            if (!presenter_->present(*frameBuffer_)) {
                reportError(
                    ApplicationError::FramePresentation,
                    presenter_->getError(),
                    "Application::run SdlPresenter::present"
                );

                stop();
            } else {
                ++fpsFrameCount;
                const Uint64 fpsCurrentTime = SDL_GetTicksNS();
                const double fpsElapsedSeconds =
                        static_cast<double>(fpsCurrentTime - fpsStartTime) * NANOSECONDS_PER_SECOND;
                if (fpsElapsedSeconds >= 1.0) {
                    const double fps = static_cast<double>(fpsFrameCount) / fpsElapsedSeconds;
                    std::cout << fps << std::endl;
                    fpsFrameCount = 0;
                    fpsStartTime = fpsCurrentTime;
                }
            }
        }

        logging::info("APP", "The main loop has stopped.");
    } catch (const std::exception &error) {
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
                if (event.window.windowID == SDL_GetWindowID(window_.get())) {
                    stop();
                    return;
                }
                break;

            case SDL_EVENT_KEY_DOWN:
                if (focused_ && event.key.key == SDLK_ESCAPE) {
                    stop();
                    return;
                }

                if (focused_) {
                    input_.handleEvent(event);
                }
                break;

            case SDL_EVENT_WINDOW_FOCUS_LOST:
                if (event.window.windowID == SDL_GetWindowID(window_.get())) {
                    focused_ = false;

                    if (!SDL_SetWindowRelativeMouseMode(window_.get(), false)) {
                        SDL_Log("Failed to release mouse: %s", SDL_GetError());
                    }

                    input_.reset();
                }
                break;

            case SDL_EVENT_WINDOW_FOCUS_GAINED:
                if (event.window.windowID == SDL_GetWindowID(window_.get())) {
                    focused_ = true;
                    input_.reset();
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if (focused_ && event.button.windowID == SDL_GetWindowID(window_.get()) &&
                    event.button.button == SDL_BUTTON_LEFT) {
                    if (!SDL_SetWindowRelativeMouseMode(window_.get(), true)) {
                        SDL_Log("Failed to capture mouse: %s", SDL_GetError());
                    }
                }
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (focused_ &&
                    event.motion.windowID == SDL_GetWindowID(window_.get()) &&
                    SDL_GetWindowRelativeMouseMode(window_.get())) {
                    input_.handleEvent(event);
                }
                break;

            default:
                if (focused_) {
                    input_.handleEvent(event);
                }
                break;
        }
    }
}

void app::Application::update(double deltaTime) {
    if (!focused_) {
        return;
    }

    playerController_.update(
        player_,
        map_,
        input_,
        deltaTime
    );
}

void app::Application::shutdown() noexcept {
    running_ = false;
    initialized_ = false;
    focused_ = false;

    input_.reset();

    // Release renderer resources before destroying the borrowed window.
    presenter_.reset();

    // Destroy objects holding references before their referenced resources.
    rayCaster_.reset();
    frameBuffer_.reset();

    window_.reset();

    if (sdlInitialized_) {
        SDL_Quit();
        sdlInitialized_ = false;
    }
}