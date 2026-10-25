#pragma once

#include <chrono>
#include <string_view>

namespace log {
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
        int code,
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
