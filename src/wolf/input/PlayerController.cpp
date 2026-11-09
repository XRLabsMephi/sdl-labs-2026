#include "wolf/input/PlayerController.hpp"
#include <cmath>

namespace wolf::input {

    PlayerController::PlayerController(double speed, double rotSpeed) : moveSpeed(speed), rotSpeed(rotSpeed) {}

    void PlayerController::update(world::Player& player, const InputState& input, const world::Map& map, double deltaTime) {
        double move_fps = moveSpeed * deltaTime;
        double rotStep = rotSpeed * deltaTime; //радианы

        //матрица поворота для направления и плоскости камеры
        if (input.turnRight || input.turnLeft) {
            double direction = 0.0;
            if (input.turnRight) {
                direction = -1.0;
            }
            if (input.turnLeft) {
                direction =  1.0;
            }
            double angle = direction * rotStep;

            double cosA = std::cos(angle);
            double sinA = std::sin(angle);

            double oldDir = player.direction.x;
            player.direction.x = player.direction.x * cosA - player.direction.y * sinA;
            player.direction.y = oldDir * sinA + player.direction.y * cosA;

            double oldPlane = player.cameraPlane.x;
            player.cameraPlane.x = player.cameraPlane.x * cosA - player.cameraPlane.y * sinA;
            player.cameraPlane.y = oldPlane * sinA + player.cameraPlane.y * cosA;
        }

        //вектор перемещения
        double moveX = 0.0;
        double moveY = 0.0;
        if (input.moveForward) {
            moveX += player.direction.x * move_fps;
            moveY += player.direction.y * move_fps;
        }
        if (input.moveBackward) {
            moveX -= player.direction.x * move_fps;
            moveY -= player.direction.y * move_fps;
        }
        if (input.moveRight) {
            moveX += player.cameraPlane.x * move_fps;
            moveY += player.cameraPlane.y * move_fps;
        }
        if (input.moveLeft) {
            moveX -= player.cameraPlane.x * move_fps;
            moveY -= player.cameraPlane.y * move_fps;
        }
        if (map.getCell(static_cast<int>(player.position.x + moveX), static_cast<int>(player.position.y)) == 0) {
            player.position.x += moveX;
        }
        if (map.getCell(static_cast<int>(player.position.x), static_cast<int>(player.position.y + moveY)) == 0) {
            player.position.y += moveY;
        }
    }
}