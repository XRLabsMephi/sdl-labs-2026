#pragma once

#include <cstdint>

namespace input {
    //Бит   Действие:
    //0:    вперёд
    //1:    назад
    //2:    влево 
    //3:    вправо
    //4:    поворот влево
    //5:    поворот вправо
    //6:    выход
    struct InputState {
        uint8_t flags = 0;

        static constexpr uint8_t kForward = 1 << 0;
        static constexpr uint8_t kBackward = 1 << 1;
        static constexpr uint8_t kLeft = 1 << 2;
        static constexpr uint8_t kRight = 1 << 3;
        static constexpr uint8_t kTurnLeft = 1 << 4;
        static constexpr uint8_t kTurnRight = 1 << 5;
        static constexpr uint8_t kQuit = 1 << 6;

        [[nodiscard]] bool isSet(uint8_t flag) const noexcept {
            return (flags & flag) != 0;
        }

        void set(uint8_t flag) noexcept {
            flags |= flag;
        }

        void clear() noexcept {
            flags = 0;
        }
    };
}
