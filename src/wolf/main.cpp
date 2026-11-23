#include <exception>
#include <iostream>

#include "wolf/app/Application.hpp"

int main() {
    try {
        app::Application app;
        return app.run();
    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << '\n';
        return 1;
    }
}
