#pragma once

#include <chrono>
#include <string>
#include <string_view>
#include <stdexcept>

namespace logging {
    enum class ErrorCode {
        SdlInitialization = 100, WindowCreation, InitializationException, NotInitialized,
        RuntimeException, MouseMode,
        ConfigOpen = 200, ConfigValue,
        MapLoading = 300, InvalidSpawn,
        PlayerRotation = 400, MovementStep, InputAxis, InputBinding,
        RayFrame = 500, RayCamera, SpriteDepthBuffer,
        FrameDimensions = 600, FrameSize,
        PresenterInitialization = 700, FramePresentation,
        TextureMapping = 800, TextureId, TextureDuplicate, TextureRead,
        LogOpen = 900, LogTime, LogWrite
    };

    class Error : public std::runtime_error {
    public:
        const ErrorCode code;

        Error(ErrorCode code, const std::string& message)
            : std::runtime_error(message), code(code) {}
    };

    void info(
        std::string_view component,
        std::string_view message
    );

    void warning(
        std::string_view component,
        std::string_view message
    );

    void error(
        std::string_view component,
        ErrorCode code,
        std::string_view message,
        std::string_view context
    );

    void enter(
        std::string_view component,
        std::string_view tool,
        std::string_view arguments = {}
    );

    void exit(
        std::string_view component,
        std::string_view tool,
        bool success,
        std::chrono::milliseconds duration
    );

    void write(
        std::string_view level,
        std::string_view component,
        std::string_view message
    );
}
