#include <cstdlib>

#include "app/Application.h"

int main(int argc, char* argv[]) {
    app::Application application;

    if (!application.initialize(argc > 1 ? argv[1] : "assets/project.cfg")) {
        return EXIT_FAILURE;
    }

    application.run();

    return EXIT_SUCCESS;
}