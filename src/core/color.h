#ifndef SOFTWAREGRAPHICSLABS_CORE_COLOR_H_
#define SOFTWAREGRAPHICSLABS_CORE_COLOR_H_

#include <cstdint>

class Color {
public:
    constexpr Color() : rgba_(MakeRgba(0, 0, 0, 255)) {}

    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
            : rgba_(MakeRgba(r, g, b, a)) {}

    // Конструктор из сырого 32-битного числа (нужен при чтении из Texture)
    explicit constexpr Color(uint32_t raw_rgba) : rgba_(raw_rgba) {}

    constexpr uint32_t GetValue() const { return rgba_; }

    // Геттеры отдельных каналов (нужны для расчета тумана и проверки прозрачности)
    constexpr uint8_t GetR() const { return static_cast<uint8_t>((rgba_ >> 24) & 0xFF); }
    constexpr uint8_t GetG() const { return static_cast<uint8_t>((rgba_ >> 16) & 0xFF); }
    constexpr uint8_t GetB() const { return static_cast<uint8_t>((rgba_ >> 8) & 0xFF); }
    constexpr uint8_t GetA() const { return static_cast<uint8_t>(rgba_ & 0xFF); }

    constexpr bool operator==(const Color& other) const {
        return rgba_ == other.rgba_;
    }
    constexpr bool operator!=(const Color& other) const {
        return rgba_ != other.rgba_;
    }

    static const Color kBlack;
    static const Color kWhite;
    static const Color kFog;
    static const Color kMagentaKey;  // Частый цвет-ключ для прозрачности в BMP

private:
    uint32_t rgba_;

    static constexpr uint32_t MakeRgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        return (static_cast<uint32_t>(r) << 24) |
               (static_cast<uint32_t>(g) << 16) |
               (static_cast<uint32_t>(b) << 8) |
               static_cast<uint32_t>(a);
    }
};


inline constexpr Color Color::kBlack{0, 0, 0};
inline constexpr Color Color::kWhite{255, 255, 255};
inline constexpr Color Color::kFog{40, 40, 45};
inline constexpr Color Color::kMagentaKey{255, 0, 255};

#endif