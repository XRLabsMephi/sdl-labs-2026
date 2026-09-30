#pragma once

#include "core/config.h"

class Application {
public:
  Application(Config config);
  ~Application();

  void Run();

private:
  Config config_;

  bool is_running_ = true;
};
