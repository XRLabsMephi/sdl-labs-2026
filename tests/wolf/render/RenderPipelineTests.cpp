#include "wolf/input/InputState.hpp"
#include "wolf/input/PlayerController.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/FrameBuffer.hpp"
#include "wolf/render/SoftwareRenderer.hpp"
#include "wolf/world/Map.hpp"
#include "wolf/world/Player.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

std::vector<wolf::Color> render(const wolf::Map& map, const wolf::Player& player) {
    wolf::FrameBuffer frame(160, 100);
    wolf::Raycaster raycaster;
    raycaster.castFrame(map, player, frame.width(), frame.height());
    require(raycaster.columns().size() == 160 && raycaster.depthBuffer().size() == 160,
            "The pipeline must cast one ray and store one depth per column");
    wolf::SoftwareRenderer::drawFrame(frame, raycaster.columns());

    const auto& center = raycaster.columns()[80];
    require(center.hit.hit && center.drawStart > 0 && center.drawEnd < frame.height(),
            "The default scene must project a wall between ceiling and floor");
    require(frame.pixelAt(80, 0) == wolf::makeColor(58, 65, 82), "Ceiling must be drawn");
    require(frame.pixelAt(80, 99) == wolf::makeColor(63, 56, 52), "Floor must be drawn");
    for (int x = 0; x < frame.width(); ++x) {
        const auto& slice = raycaster.columns()[static_cast<std::size_t>(x)];
        require(slice.drawStart >= 0 && slice.drawEnd <= frame.height(),
                "Projected walls must stay inside the frame");
        require(slice.hit.hit && std::isfinite(raycaster.depthBuffer()[static_cast<std::size_t>(x)]),
                "The enclosed map must have a finite first wall in every column");
        for (int y = slice.drawStart; y < slice.drawEnd; ++y) {
            const auto pixel = frame.pixelAt(x, y);
            require(pixel != wolf::makeColor(58, 65, 82) && pixel != wolf::makeColor(63, 56, 52),
                    "Every projected wall pixel must replace the background");
        }
        if (slice.drawEnd < frame.height()) {
            require(frame.pixelAt(x, slice.drawEnd) == wolf::makeColor(63, 56, 52),
                    "drawEnd must be exclusive");
        }
    }
    return {frame.pixels(), frame.pixels() + frame.width() * frame.height()};
}

void testScenePipeline() {
    const wolf::Map map = wolf::makeDefaultMap();
    wolf::Player player;
    const auto initial = render(map, player);
    wolf::InputState input;
    input.moveForward = true;
    wolf::PlayerController::update(player, map, input, 0.1);
    const auto moved = render(map, player);
    require(initial != moved, "Movement must change the rendered scene");
    input.moveForward = false;
    input.turnRight = true;
    wolf::PlayerController::update(player, map, input, 0.1);
    require(moved != render(map, player), "Turning must change the rendered scene");
}

} // namespace

int main() {
    try {
        testScenePipeline();
        std::cout << "All render pipeline tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
