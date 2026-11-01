#include "core/application.h"

int main(int argc, char *argv[]) {
  Application app(Config::Parse(argc, argv));
  app.Run();
  return 0;
}
