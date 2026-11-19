#include <app/Application.h>
#include <config/ProjectSettings.h>
#include <debug/Logger.h>
#include <render/TextureStorageLoader.h>

#include <algorithm>
#include <exception>
#include <filesystem>
#include <iostream>
#include <string>
#include <numbers>

namespace {

constexpr double NANOSECONDS_PER_SECOND = 1e-9;
using logging::ErrorCode;

// Preserve the original error even if writing to the log file fails.
void reportError(
    ErrorCode code,
    const std::string& message,
    const std::string& context
) noexcept {
    try {
        constexpr std::string_view components[] = {
            "APP", "APP", "CONFIG", "MAP", "PLAYER", "PROJECT", "FRAME", "PRSNTR", "TEXTURE", "LOGGER"
        };
        logging::error(
            components[static_cast<int>(code) / 100],
            code,
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

bool app::Application::initialize(const std::string& settingsFilename) {
    if (initialized_) {
        return true;
    }

    try {
        logging::info("APP", "Initializing application.");

        namespace fs = std::filesystem;
        const char* base = SDL_GetBasePath();
        const fs::path root = base ? fs::path(base) : fs::current_path();
        const auto settings = config::load(root / settingsFilename);
        const fs::path mapPath = root / settings.map;
        player_.setRadius(settings.player_radius);
        relativeMouse_ = settings.relative_mouse;
        showFps_ = settings.show_fps;

        if (!map_.loadFromFile(mapPath.string())) {
            reportError(
                ErrorCode::MapLoading,
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
                ErrorCode::InvalidSpawn,
                "The player cannot occupy the spawn position.",
                "Application::initialize"
            );

            return false;
        }

        player_.setPosition(map_.getSpawnPosition());
        player_.setEyeHeight(settings.eye_height);
        player_.setAngle(settings.field_of_view_deg * std::numbers::pi / 180.0);

        // Initialize all controller settings before the first update.
        playerController_.setMoveSpeed(settings.move_speed);
        playerController_.setRotationSpeed(settings.rotation_speed);
        playerController_.setMouseSensitivity(settings.mouse_sensitivity);

        playerController_.setMaxMovementStep(settings.max_movement_step);

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            reportError(
                ErrorCode::SdlInitialization,
                SDL_GetError(),
                "Application::initialize SDL_Init"
            );

            return false;
        }

        sdlInitialized_ = true;

        std::array<SDL_Scancode, static_cast<std::size_t>(input::Action::Count)> keys{};
        const auto scancode = [](const std::string& name) {
            const auto key = SDL_GetScancodeFromName(name.c_str());
            if (key == SDL_SCANCODE_UNKNOWN) {
                throw logging::Error(ErrorCode::InputBinding, "Unknown key: " + name);
            }
            return key;
        };
        exitKey_ = scancode(settings.exit_key);
        for (std::size_t i = 0; i < keys.size(); ++i) {
            keys[i] = scancode(settings.movement_keys[i]);
            if (keys[i] == exitKey_ || std::find(keys.begin(), keys.begin() + i, keys[i]) != keys.begin() + i) {
                throw logging::Error(ErrorCode::InputBinding, "Duplicate key: " + settings.movement_keys[i]);
            }
        }
        input_.setBindings(keys);

        window_.reset(
            SDL_CreateWindow(
                settings.title.c_str(),
                settings.window_width,
                settings.window_height,
                settings.resizable ? SDL_WINDOW_RESIZABLE : 0
            )
        );

        if (!window_) {
            reportError(
                ErrorCode::WindowCreation,
                SDL_GetError(),
                "Application::initialize SDL_CreateWindow"
            );

            shutdown();
            return false;
        }

        if (relativeMouse_ && !SDL_SetWindowRelativeMouseMode(window_.get(), true)) {
            reportError(ErrorCode::MouseMode, SDL_GetError(), "Enable relative mouse mode");
        }

        render::TextureStorageLoader(
            root / settings.texture_mapping
            ).load(textureStorage_);

        frameBuffer_ = std::make_unique<render::FrameBuffer>(
            settings.frame_width,
            settings.frame_height
        );

        frameBuffer_->set_fog_params(settings.fog_color, settings.fog_start, settings.fog_end);

        viewSettings_ = settings.view;
        // RayCaster borrows the player, map, frame buffer and textures.
        rayCaster_ = std::make_unique<raycast::RayCaster>(
            player_,
            map_,
            *frameBuffer_,
            textureStorage_,
            viewSettings_
        );

        rayCaster_->set_fog_params(settings.fog_color, settings.fog_start, settings.fog_end);

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
            window_.get(), settings.frame_width, settings.frame_height
        );

        const std::string presenterError = presenter_->getError();

        if (!presenterError.empty()) {
            reportError(
                ErrorCode::PresenterInitialization,
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
    } catch (const logging::Error& error) {
        reportError(error.code, error.what(), "Application::initialize");
        shutdown();
        return false;
    } catch (const std::exception &error) {
        reportError(
            ErrorCode::InitializationException,
            error.what(),
            "Application::initialize"
        );

        shutdown();
        return false;
    } catch (...) {
        reportError(
            ErrorCode::InitializationException,
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
            ErrorCode::NotInitialized,
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
                    ErrorCode::FramePresentation,
                    presenter_->getError(),
                    "Application::run SdlPresenter::present"
                );

                stop();
            } else if (showFps_) {
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
    } catch (const logging::Error& error) {
        reportError(error.code, error.what(), "Application::run");
    } catch (const std::exception &error) {
        reportError(
            ErrorCode::RuntimeException,
            error.what(),
            "Application::run"
        );
    } catch (...) {
        reportError(
            ErrorCode::RuntimeException,
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
                if (focused_ && event.key.scancode == exitKey_) {
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
                        reportError(ErrorCode::MouseMode, SDL_GetError(), "Release relative mouse mode");
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
                    if (relativeMouse_ && !SDL_SetWindowRelativeMouseMode(window_.get(), true)) {
                        reportError(ErrorCode::MouseMode, SDL_GetError(), "Capture relative mouse mode");
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
    spriteRenderer_.reset();
    rayCaster_.reset();
    frameBuffer_.reset();

    window_.reset();

    if (sdlInitialized_) {
        SDL_Quit();
        sdlInitialized_ = false;
    }
}