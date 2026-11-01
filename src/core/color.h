#ifndef SOFTWAREGRAPHICSLABS_CORE_COLOR_H_
#define SOFTWAREGRAPHICSLABS_CORE_COLOR_H_

#include <cstdint>

class Color {
public:
    // По умолчанию черный
    constexpr Color() : rgba_(MakeRgba(0, 0, 0, 255)) {}

    // Основной конструктор для задания цвета (RGBA)
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : rgba_(MakeRgba(r, g, b, a)) {}

    // Геттер сырого значения для FrameBuffer
    constexpr uint32_t GetValue() const { return rgba_; }

    static const Color kRed;
    static const Color kGreen;
    static const Color kBlue;
    static const Color kBlack;
    static const Color kWhite;


private:
    uint32_t rgba_;

    // Упаковка в 32 бита: формат RGBA8888
    static constexpr uint32_t MakeRgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        return (static_cast<uint32_t>(r) << 24) |
               (static_cast<uint32_t>(g) << 16) |
               (static_cast<uint32_t>(b) << 8) |
               static_cast<uint32_t>(a);
    }
};

inline constexpr Color Color::kRed{255, 0, 0};
inline constexpr Color Color::kGreen{0, 255, 0};
inline constexpr Color Color::kBlue{0, 0, 255};
inline constexpr Color Color::kBlack{0, 0, 0};
inline constexpr Color Color::kWhite{255, 255, 255};


#endif  // SOFTWAREGRAPHICSLABS_CORE_COLOR_H_