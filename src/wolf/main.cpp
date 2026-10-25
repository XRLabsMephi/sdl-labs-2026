#include <cstdlib>

#include "app/Application.h"

int main() {
    app::Application application;

    if (!application.initialize("assets/map.txt")) {
        return EXIT_FAILURE;
    }

    application.run();

    return EXIT_SUCCESS;
}