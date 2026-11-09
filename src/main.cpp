#include "wolf/app/Application.hpp"

#include <exception>
#include <iostream>

int main()
{
    try
    {
        return wolf::Application{}.run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Application error: " << error.what() << '\n';
        return 1;
    }
}
