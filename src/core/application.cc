#include "application.h"

Application::Application(Config config) : config_(config), is_running_(true) {}

Application::~Application() {}

void Application::Run() {
  while (is_running_) {
    // TODO: Implement the main application loop
  }
}
