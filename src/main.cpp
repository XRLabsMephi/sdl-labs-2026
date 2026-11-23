#include "wolf/app/Application.hpp"

#include <exception>
#include <iostream>
#include <string_view>

int main(int argc, char* argv[])
{
    if (argc > 2 || (argc == 2 && std::string_view(argv[1]) != "--lab1"))
    {
        std::cerr << "Usage: graphics_lab [--lab1]\n";
        return 2;
    }
    try
    {
        return wolf::Application{}.run(argc == 1);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Application error: " << error.what() << '\n';
        return 1;
    }
}
