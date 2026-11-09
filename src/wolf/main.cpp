#include "app/Application.hpp"

int main() {
    try {
        Application application;
        application.run();
    } catch (const std::runtime_error& e) {
        std::cerr << "Critical application error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }


    return 0;
}