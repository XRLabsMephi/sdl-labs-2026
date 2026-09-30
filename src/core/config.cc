#include "config.h"

Config Config::Parse(int argc, char* argv[]) {
  Config config;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--width" && i + 1 < argc) {
      config.width = std::stoi(argv[++i]);
    } else if (arg == "--height" && i + 1 < argc) {
      config.height = std::stoi(argv[++i]);
    } else if (arg == "--fullscreen") {
      config.screen_mode = ScreenMode::Fullscreen;
    } else if (arg == "--windowed") {
      config.screen_mode = ScreenMode::Windowed;
    } else if (arg == "--map" && i + 1 < argc) {
      config.map_file = argv[++i];
    }
  }
  return config;
}
