#include "../../../include/wolf/debug/Logger.h"

#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

constexpr int LEVEL_WIDTH = 8;
constexpr int COMPONENT_WIDTH = 9;

constexpr std::size_t MAX_COMPONENT_LENGTH = 8;

constexpr char DATE_FORMAT[] = "%Y-%m-%d";
constexpr char TIME_FORMAT[] = "%H:%M:%S";

std::ofstream& getLogFile() {
    static std::ofstream file("log.txt", std::ios::app);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open log.txt");
    }

    return file;
}

std::string singleLine(std::string_view text) {
    std::string result(text);

    for (char& ch : result) {
        if (ch == '\n' || ch == '\r' || ch == '\t') {
            ch = ' ';
        }
    }

    return result;
}

}

void logging::write(
    std::string_view level,
    std::string_view component,
    std::string_view message
) {
    std::ofstream& file = getLogFile();

    static bool sessionStarted = false;

    std::time_t now = std::time(nullptr);
    const std::tm* localTime = std::localtime(&now);

    if (localTime == nullptr) {
        throw std::runtime_error("Failed to obtain local time");
    }

    std::tm time = *localTime;

    std::ostringstream output;

    if (!sessionStarted) {
        output << "=== LOG SESSION STARTED: "
               << std::put_time(&time, DATE_FORMAT)
               << " ===\n\n";
    }

    std::string tag = singleLine(
        component.substr(0, MAX_COMPONENT_LENGTH)
    );

    output << std::put_time(&time, TIME_FORMAT) << ' '
           << std::left
           << std::setw(LEVEL_WIDTH) << level
           << std::setw(COMPONENT_WIDTH) << tag
           << singleLine(message)
           << '\n';

    file << output.str();
    file.flush();

    if (!file) {
        throw std::runtime_error("Failed to write to log.txt");
    }

    sessionStarted = true;
}

void logging::info(
    std::string_view component,
    std::string_view message
) {
    write("INFO", component, message);
}

void logging::warning(
    std::string_view component,
    std::string_view message
) {
    write("WARNING", component, message);
}

void logging::error(
    std::string_view component,
    int code,
    std::string_view message,
    std::string_view context
) {
    std::ostringstream text;

    text << "==>>> ERROR (code="
         << std::dec << code
         << ", 0x" << std::hex << code
         << "): Msg: " << message
         << " | Context: " << context;

    write("ERROR", component, text.str());
}

void logging::enter(
    std::string_view component,
    std::string_view tool,
    std::string_view arguments
) {
    std::ostringstream text;

    text << "==>>> Entering " << tool << ':';

    if (!arguments.empty()) {
        text << ' ' << arguments;
    }

    write("ENTER", component, text.str());
}

void logging::exit(
    std::string_view component,
    std::string_view tool,
    bool success,
    std::chrono::milliseconds duration
) {
    std::ostringstream text;

    text << "<<<== EXIT SideTool: " << tool
         << " | Status: " << (success ? "Success" : "Fail")
         << " | Duration: " << duration.count()
         << "ms";

    write("EXIT", component, text.str());
}