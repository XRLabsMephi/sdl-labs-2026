#include "core/application.h"

#include <exception>
#include <iostream>

int main(int argc, char *argv[]) {
  try {
    Application app(Config::Parse(argc, argv));
    app.Run();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "Wolf: " << error.what() << '\n';
    return 1;
  }
}
