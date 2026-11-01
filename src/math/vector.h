#pragma once

#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <type_traits>

template <typename T, std::size_t N>
  requires(std::is_arithmetic_v<T> && N > 0)
class Vector {
public:
  constexpr Vector() : data_{} {}

  constexpr explicit Vector(const std::array<T, N> &data) : data_(data) {}

  template <typename... Args>
    requires(sizeof...(Args) == N)
  constexpr Vector(Args... args) : data_{static_cast<T>(args)...} {}

  // Аксессоры
  constexpr const T &x() const { return data_[0]; }
  constexpr T &x() { return data_[0]; }

  constexpr const T &y() const
    requires(N >= 2)
  {
    return data_[1];
  }
  constexpr T &y()
    requires(N >= 2)
  {
    return data_[1];
  }

  constexpr const T &z() const
    requires(N >= 3)
  {
    return data_[2];
  }
  constexpr T &z()
    requires(N >= 3)
  {
    return data_[2];
  }

  constexpr const T &w() const
    requires(N >= 4)
  {
    return data_[3];
  }
  constexpr T &w()
    requires(N >= 4)
  {
    return data_[3];
  }

  // Доступ к данным
  constexpr T &operator[](std::size_t index) { return data_[index]; }
  constexpr const T &operator[](std::size_t index) const {
    return data_[index];
  }

  [[nodiscard]] constexpr const T *data() const noexcept {
    return data_.data();
  }
  [[nodiscard]] constexpr T *data() noexcept { return data_.data(); }
  [[nodiscard]] static constexpr std::size_t size() noexcept { return N; }

  // Итераторы для range-based for и std::ranges
  [[nodiscard]] constexpr auto begin() noexcept { return data_.begin(); }
  [[nodiscard]] constexpr auto end() noexcept { return data_.end(); }
  [[nodiscard]] constexpr auto begin() const noexcept { return data_.begin(); }
  [[nodiscard]] constexpr auto end() const noexcept { return data_.end(); }

  // Приведение типа: Vec2f -> Vec2i
  template <typename U>
    requires std::is_arithmetic_v<U>
  [[nodiscard]] constexpr Vector<U, N> As() const {
    Vector<U, N> result;
    for (std::size_t i = 0; i < N; ++i) {
      result[i] = static_cast<U>(data_[i]);
    }
    return result;
  }

  // Покомпонентные операции (вектор-вектор)
  constexpr Vector<T, N> &operator+=(const Vector<T, N> &other) {
    for (std::size_t i = 0; i < N; ++i)
      data_[i] += other[i];
    return *this;
  }

  constexpr Vector<T, N> operator+(const Vector<T, N> &other) const {
    Vector<T, N> result = *this;
    result += other;
    return result;
  }

  constexpr Vector<T, N> &operator-=(const Vector<T, N> &other) {
    for (std::size_t i = 0; i < N; ++i)
      data_[i] -= other[i];
    return *this;
  }

  constexpr Vector<T, N> operator-(const Vector<T, N> &other) const {
    Vector<T, N> result = *this;
    result -= other;
    return result;
  }

  constexpr Vector<T, N> operator-() const {
    Vector<T, N> result;
    for (std::size_t i = 0; i < N; ++i)
      result[i] = -data_[i];
    return result;
  }

  constexpr Vector<T, N> &operator*=(const Vector<T, N> &other) {
    for (std::size_t i = 0; i < N; ++i)
      data_[i] *= other[i];
    return *this;
  }

  constexpr Vector<T, N> operator*(const Vector<T, N> &other) const {
    Vector<T, N> result = *this;
    result *= other;
    return result;
  }

  constexpr Vector<T, N> &operator/=(const Vector<T, N> &other) {
    for (std::size_t i = 0; i < N; ++i)
      data_[i] /= other[i];
    return *this;
  }

  constexpr Vector<T, N> operator/(const Vector<T, N> &other) const {
    Vector<T, N> result = *this;
    result /= other;
    return result;
  }

  // Операции со скалярами
  template <typename U>
    requires std::is_arithmetic_v<U>
  constexpr Vector<T, N> &operator*=(U scalar) {
    for (std::size_t i = 0; i < N; ++i)
      data_[i] = static_cast<T>(data_[i] * scalar);
    return *this;
  }

  template <typename U>
    requires std::is_arithmetic_v<U>
  constexpr Vector<T, N> operator*(U scalar) const {
    Vector<T, N> result = *this;
    result *= scalar;
    return result;
  }

  template <typename U>
    requires std::is_arithmetic_v<U>
  friend constexpr Vector<T, N> operator*(U scalar, const Vector<T, N> &vec) {
    return vec * scalar;
  }

  template <typename U>
    requires std::is_arithmetic_v<U>
  constexpr Vector<T, N> &operator/=(U scalar) {
    for (std::size_t i = 0; i < N; ++i)
      data_[i] = static_cast<T>(data_[i] / scalar);
    return *this;
  }

  template <typename U>
    requires std::is_arithmetic_v<U>
  constexpr Vector<T, N> operator/(U scalar) const {
    Vector<T, N> result = *this;
    result /= scalar;
    return result;
  }

  template <typename U>
    requires std::is_arithmetic_v<U>
  friend constexpr Vector<T, N> operator/(U scalar, const Vector<T, N> &vec) {
    Vector<T, N> result;
    for (std::size_t i = 0; i < N; ++i)
      result[i] = static_cast<T>(scalar) / vec[i];
    return result;
  }

  bool operator==(const Vector<T, N> &) const = default;

  // Скалярное произведение и длины
  [[nodiscard]] constexpr T Dot(const Vector<T, N> &other) const {
    T result = T();
    for (std::size_t i = 0; i < N; ++i)
      result += data_[i] * other[i];
    return result;
  }

  [[nodiscard]] constexpr T LengthSquared() const { return Dot(*this); }

  [[nodiscard]] auto Length() const {
    if constexpr (std::floating_point<T>) {
      return std::sqrt(LengthSquared());
    } else {
      return std::sqrt(static_cast<double>(LengthSquared()));
    }
  }

  [[nodiscard]] constexpr T DistanceSquared(const Vector<T, N> &other) const {
    return (*this - other).LengthSquared();
  }

  [[nodiscard]] auto Distance(const Vector<T, N> &other) const {
    return (*this - other).Length();
  }

  // Нормализация
  Vector<T, N> &Normalize()
    requires std::floating_point<T>
  {
    T length = Length();
    if (length > T()) {
      *this /= length;
    }
    return *this;
  }

  [[nodiscard]] Vector<T, N> Normalized() const
    requires std::floating_point<T>
  {
    Vector<T, N> result = *this;
    result.Normalize();
    return result;
  }

  // Поворот на 90 градусов (ортогональный вектор: стрейф игрока, камера)
  [[nodiscard]] constexpr Vector<T, 2> Perpendicular() const
    requires(N == 2)
  {
    return Vector<T, 2>{-data_[1], data_[0]};
  }

  // Поворот на произвольный угол (без constexpr, так как std::sin/cos не
  // constexpr в C++20)
  [[nodiscard]] Vector<T, 2> Rotated(T angle_radians) const
    requires(N == 2 && std::floating_point<T>)
  {
    const T cos_a = std::cos(angle_radians);
    const T sin_a = std::sin(angle_radians);
    return Vector<T, 2>{data_[0] * cos_a - data_[1] * sin_a,
                        data_[0] * sin_a + data_[1] * cos_a};
  }

  // Векторные произведения
  [[nodiscard]] constexpr T Cross(const Vector<T, N> &other) const
    requires(N == 2)
  {
    return data_[0] * other[1] - data_[1] * other[0];
  }

  [[nodiscard]] constexpr Vector<T, N> Cross(const Vector<T, N> &other) const
    requires(N == 3)
  {
    return Vector<T, N>{data_[1] * other[2] - data_[2] * other[1],
                        data_[2] * other[0] - data_[0] * other[2],
                        data_[0] * other[1] - data_[1] * other[0]};
  }

  [[nodiscard]] constexpr Vector<T, N> Cross(const Vector<T, N> &other) const
    requires(N == 7)
  {
    return Vector<T, N>{data_[1] * other[2] - data_[2] * other[1],
                        data_[2] * other[0] - data_[0] * other[2],
                        data_[0] * other[1] - data_[1] * other[0],
                        data_[3] * other[4] - data_[4] * other[3],
                        data_[4] * other[5] - data_[5] * other[4],
                        data_[5] * other[6] - data_[6] * other[5],
                        data_[6] * other[3] - data_[3] * other[6]};
  }

private:
  std::array<T, N> data_;
};

using Vec2f = Vector<float, 2>;
using Vec2i = Vector<int, 2>;
