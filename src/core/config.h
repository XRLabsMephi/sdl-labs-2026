#pragma once

#include <string>

struct Config {
  static Config Parse(int argc, char* argv[]);

  int width = 800;
  int height = 600;
  enum class ScreenMode {
    Windowed,
    Fullscreen
  } screen_mode = ScreenMode::Windowed;
  std::string map_file = "template.map";
};
