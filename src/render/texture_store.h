#ifndef SOFTWAREGRAPHICSLABS_RENDER_TEXTURE_STORE_H_
#define SOFTWAREGRAPHICSLABS_RENDER_TEXTURE_STORE_H_

#include <cstddef>
#include <string>
#include <unordered_map>

#include "render/texture.h"

class TextureStore {
 public:
  TextureStore() = default;

  // Регистрирует готовую текстуру с заданным числовым id.
  // Перезаписывает существующую текстуру с таким же id.
  void Register(int id, Texture texture);

  // Регистрирует готовую текстуру с автоматически назначенным id.
  // Возвращает назначенный id.
  int Register(Texture texture);

  // Загружает BMP файл с диска и регистрирует его под id.
  // Выбрасывает std::runtime_error при ошибке загрузки или конвертации.
  void Load(int id, const std::string& filepath);

  // Попытка загрузить BMP файл. Возвращает true при успехе, false при ошибке.
  bool TryLoad(int id, const std::string& filepath) noexcept;

  // Загрузка BMP файла в Texture через SDL3 без регистрации в хранилище.
  // Выбрасывает std::runtime_error при ошибке.
  [[nodiscard]] static Texture LoadFromFile(const std::string& filepath);

  // Сохранение текстуры в BMP файл через SDL3.
  static bool SaveToBmp(const Texture& texture, const std::string& filepath);

  // Доступ к текстуре по id. Если id не найден, возвращает fallback-текстуру (если задана)
  // либо выбрасывает std::out_of_range.
  [[nodiscard]] const Texture& Get(int id) const;

  // Безопасный поиск текстуры. Возвращает указатель на текстуру или nullptr, если id не найден.
  [[nodiscard]] const Texture* Find(int id) const noexcept;

  // Проверка наличия текстуры с данным id.
  [[nodiscard]] bool Has(int id) const noexcept;

  // Задать fallback-текстуру, возвращаемую методом Get при отсутствии запрошенного id.
  void SetFallback(Texture fallback);

  // Сбросить fallback-текстуру.
  void ClearFallback() noexcept;

  // Проверка наличия fallback-текстуры.
  [[nodiscard]] bool HasFallback() const noexcept;

  // Количество зарегистрированных текстур в хранилище.
  [[nodiscard]] size_t Size() const noexcept;

  // Проверка на пустоту хранилища.
  [[nodiscard]] bool Empty() const noexcept;

  // Очистить все зарегистрированные текстуры.
  void Clear() noexcept;

  // Фабричные методы для процедурной генерации текстур.
  [[nodiscard]] static Texture CreateCheckerboard(int width, int height,
                                                  int check_size, Color color1,
                                                  Color color2);
  [[nodiscard]] static Texture CreateSolid(int width, int height, Color color);

 private:
  std::unordered_map<int, Texture> textures_;
  Texture fallback_;
  bool has_fallback_ = false;
  int next_auto_id_ = 1000;
};

#endif  // SOFTWAREGRAPHICSLABS_RENDER_TEXTURE_STORE_H_
