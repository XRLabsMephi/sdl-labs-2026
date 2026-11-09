#include "app/Application.hpp"

int main(int argc, char* argv[]) {
    try {
        Application application;
        application.run();
    } catch (const std::runtime_error& e) {
        std::cerr << "Critical application error: " << e.what() << "\n";
        return EXIT_FAILURE;
    }


    return 0;
}