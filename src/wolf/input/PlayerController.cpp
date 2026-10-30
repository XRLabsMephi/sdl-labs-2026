#include <input/PlayerController.h>

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

constexpr double MAX_MOVEMENT_STEP = 0.1;

}

void input::PlayerController::update(
    world::Player& player,
    const world::Map& map,
    const InputState& input,
    double deltaTime
) const {
    if (!std::isfinite(deltaTime) || deltaTime < 0.0) {
        return;
    }

    updateRotation(player, input, deltaTime);

    const core::Vector2 displacement =
        calculateMovement(player, input, deltaTime);

    tryMove(player, map, displacement);
}

core::Vector2 input::PlayerController::calculateMovement(
    const world::Player& player,
    const InputState& input,
    double deltaTime
) const {
    const double forwardInput =
        static_cast<double>(input.isActionActive(Action::MoveForward))
        - static_cast<double>(input.isActionActive(Action::MoveBackward));

    const double sideInput =
        static_cast<double>(input.isActionActive(Action::MoveRight))
        - static_cast<double>(input.isActionActive(Action::MoveLeft));

    const double inputLength = std::hypot(forwardInput, sideInput);

    if (inputLength == 0.0) {
        return {};
    }

    // Normalize input so diagonal movement has the same speed.
    const double forwardAmount = forwardInput / inputLength;
    const double sideAmount = sideInput / inputLength;

    const core::Vector2 forward = player.getDirection();

    // Match the horizontal orientation used by RayCaster's camera plane.
    const core::Vector2 right{
        forward.y,
        -forward.x
    };

    const double distance = moveSpeed * deltaTime;

    return {
        (forward.x * forwardAmount + right.x * sideAmount) * distance,
        (forward.y * forwardAmount + right.y * sideAmount) * distance
    };
}

void input::PlayerController::tryMove(
    world::Player& player,
    const world::Map& map,
    core::Vector2 displacement
) const {
    const double distance = std::hypot(
        displacement.x,
        displacement.y
    );

    if (!std::isfinite(distance) || distance == 0.0) {
        return;
    }

    const double radius = player.getRadius();

    const double stepLength = radius > 0.0
        ? std::min(MAX_MOVEMENT_STEP, radius)
        : MAX_MOVEMENT_STEP;

    const double requiredSteps = std::ceil(distance / stepLength);

    // Validate the step count before converting it to an integer.
    if (!std::isfinite(requiredSteps) ||
        requiredSteps > std::numeric_limits<int>::max()) {
        return;
    }

    const int steps = static_cast<int>(requiredSteps);

    const core::Vector2 step{
        displacement.x / steps,
        displacement.y / steps
    };

    core::Vector2 position = player.getPosition();

    for (int i = 0; i < steps; ++i) {
        bool moved = false;

        // Check each axis separately to allow sliding along walls.
        core::Vector2 candidate{
            position.x + step.x,
            position.y
        };

        if (step.x != 0.0 && map.canOccupy(candidate, radius)) {
            position = candidate;
            moved = true;
        }

        candidate = {
            position.x,
            position.y + step.y
        };

        if (step.y != 0.0 && map.canOccupy(candidate, radius)) {
            position = candidate;
            moved = true;
        }

        // Further identical steps cannot move a fully blocked player.
        if (!moved) {
            break;
        }
    }

    player.setPosition(position);
}

void input::PlayerController::updateRotation(
    world::Player& player,
    const InputState& input,
    double deltaTime
) const {
    const double keyboardDirection =
        static_cast<double>(input.isActionActive(Action::TurnLeft))
        - static_cast<double>(input.isActionActive(Action::TurnRight));

    const double keyboardRotation =
        keyboardDirection * rotationSpeed * deltaTime;

    // Mouse motion is already accumulated over the frame.
    const double mouseRotation =
        -input.getMouseDeltaX() * mouseSensitivity;

    const double rotation = keyboardRotation + mouseRotation;

    if (std::isfinite(rotation) && rotation != 0.0) {
        player.rotate(rotation);
    }
}