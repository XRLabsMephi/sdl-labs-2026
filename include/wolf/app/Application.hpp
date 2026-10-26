#pragma once

#include "wolf/input/PlayerController.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <functional>

struct SDL_Window;

namespace wolf
{

class Application
{
public:
    using RenderFrame = std::function<void(const Map&, const Player&)>;
    // Called once after window creation. Resources owned by the returned
    // closure are released before the window is destroyed and SDL shuts down.
    using RendererFactory = std::function<RenderFrame(SDL_Window&)>;

    // Without a renderer, opens a diagnostic window with camera data in its title.
    explicit Application(RendererFactory rendererFactory = {});
    int run();

    [[nodiscard]] const Map& map() const noexcept;
    [[nodiscard]] const Player& player() const noexcept;

private:
    Map map_;
    Player player_;
    PlayerController controller_;
    RendererFactory rendererFactory_;
};

}
