#pragma once
#pragma once

#include <chrono>
#include <string_view>

namespace core {

    class Logger {
    public:
        static void info(
            std::string_view component,
            std::string_view message
        );

        static void warning(
            std::string_view component,
            std::string_view message
        );

        static void error(
            std::string_view component,
            int code,
            std::string_view message,
            std::string_view context
        );

        static void enter(
            std::string_view component,
            std::string_view tool,
            std::string_view arguments = {}
        );

        static void exit(
            std::string_view component,
            std::string_view tool,
            bool success,
            std::chrono::milliseconds duration
        );

    private:
        static void write(
            std::string_view level,
            std::string_view component,
            std::string_view message
        );
    };

}
