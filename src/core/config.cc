#include "config.h"

#include <stdexcept>

namespace {
int ParseDimension(const std::string& value) {
  size_t end = 0;
  const int result = std::stoi(value, &end);
  if (end != value.size() || result <= 0 || result > 8192) {
    throw std::invalid_argument("Window dimensions must be in [1, 8192]");
  }
  return result;
}
}

Config Config::Parse(int argc, char* argv[]) {
  Config config;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if ((arg == "--width" || arg == "--height" || arg == "--map") &&
        i + 1 >= argc) {
      throw std::invalid_argument("Missing value for " + arg);
    }
    if (arg == "--width") {
      config.width = ParseDimension(argv[++i]);
    } else if (arg == "--height") {
      config.height = ParseDimension(argv[++i]);
    } else if (arg == "--fullscreen") {
      config.screen_mode = ScreenMode::Fullscreen;
    } else if (arg == "--windowed") {
      config.screen_mode = ScreenMode::Windowed;
    } else if (arg == "--map") {
      config.map_file = argv[++i];
    } else {
      throw std::invalid_argument("Unknown argument: " + arg);
    }
  }
  if (static_cast<size_t>(config.width) * config.height > 16 * 1024 * 1024) {
    throw std::invalid_argument("Frame exceeds 16 megapixels");
  }
  return config;
}
