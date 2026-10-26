#include "wolf/app/Application.hpp"

#include <SDL3/SDL_main.h>

int main(int, char*[])
{
    wolf::Application app;
    return app.run();
}
