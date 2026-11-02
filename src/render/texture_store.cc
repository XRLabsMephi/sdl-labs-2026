#include "render/texture_store.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <stdexcept>
#include <utility>

void TextureStore::Register(int id, Texture texture) {
  textures_[id] = std::move(texture);
  if (id >= next_auto_id_) {
    next_auto_id_ = id + 1;
  }
}

int TextureStore::Register(Texture texture) {
  while (textures_.contains(next_auto_id_)) {
    ++next_auto_id_;
  }
  const int assigned_id = next_auto_id_++;
  textures_[assigned_id] = std::move(texture);
  return assigned_id;
}

void TextureStore::Load(int id, const std::string& filepath) {
  Register(id, LoadFromFile(filepath));
}

bool TextureStore::TryLoad(int id, const std::string& filepath) noexcept {
  try {
    Load(id, filepath);
    return true;
  } catch (...) {
    return false;
  }
}

Texture TextureStore::LoadFromFile(const std::string& filepath) {
  SDL_Surface* surface = SDL_LoadBMP(filepath.c_str());
  if (surface == nullptr) {
    throw std::runtime_error("Failed to load BMP file '" + filepath +
                             "': " + SDL_GetError());
  }

  SDL_Surface* converted =
      SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA8888);
  SDL_DestroySurface(surface);

  if (converted == nullptr) {
    throw std::runtime_error(
        "Failed to convert surface to RGBA8888 for '" + filepath +
        "': " + SDL_GetError());
  }

  const int w = converted->w;
  const int h = converted->h;

  if (w <= 0 || h <= 0) {
    SDL_DestroySurface(converted);
    throw std::runtime_error("Invalid texture dimensions in '" + filepath + "'");
  }

  std::vector<uint32_t> pixels(static_cast<size_t>(w * h));
  for (int y = 0; y < h; ++y) {
    const uint32_t* row = reinterpret_cast<const uint32_t*>(
        static_cast<const uint8_t*>(converted->pixels) + y * converted->pitch);
    std::copy_n(row, w, pixels.data() + y * w);
  }

  SDL_DestroySurface(converted);
  return Texture(w, h, std::move(pixels));
}

bool TextureStore::SaveToBmp(const Texture& texture,
                             const std::string& filepath) {
  if (!texture.IsValid()) {
    return false;
  }

  SDL_Surface* surface = SDL_CreateSurface(
      texture.width, texture.height, SDL_PIXELFORMAT_RGBA8888);
  if (surface == nullptr) {
    return false;
  }

  for (int y = 0; y < texture.height; ++y) {
    uint32_t* row = reinterpret_cast<uint32_t*>(
        static_cast<uint8_t*>(surface->pixels) + y * surface->pitch);
    std::copy_n(texture.pixels.data() + y * texture.width, texture.width, row);
  }

  const bool success = SDL_SaveBMP(surface, filepath.c_str());
  SDL_DestroySurface(surface);
  return success;
}

const Texture& TextureStore::Get(int id) const {
  const auto it = textures_.find(id);
  if (it != textures_.end()) {
    return it->second;
  }
  if (has_fallback_) {
    return fallback_;
  }
  throw std::out_of_range("Texture not found with id: " + std::to_string(id));
}

const Texture* TextureStore::Find(int id) const noexcept {
  const auto it = textures_.find(id);
  if (it != textures_.end()) {
    return &it->second;
  }
  return nullptr;
}

bool TextureStore::Has(int id) const noexcept {
  return textures_.contains(id);
}

void TextureStore::SetFallback(Texture fallback) {
  fallback_ = std::move(fallback);
  has_fallback_ = true;
}

void TextureStore::ClearFallback() noexcept {
  fallback_ = Texture();
  has_fallback_ = false;
}

bool TextureStore::HasFallback() const noexcept {
  return has_fallback_;
}

size_t TextureStore::Size() const noexcept {
  return textures_.size();
}

bool TextureStore::Empty() const noexcept {
  return textures_.empty();
}

void TextureStore::Clear() noexcept {
  textures_.clear();
}

Texture TextureStore::CreateCheckerboard(int width, int height, int check_size,
                                         Color color1, Color color2) {
  if (width <= 0 || height <= 0 || check_size <= 0) {
    return Texture();
  }

  std::vector<uint32_t> pixels(static_cast<size_t>(width * height));
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const bool check = ((x / check_size) + (y / check_size)) % 2 == 0;
      pixels[static_cast<size_t>(y * width + x)] =
          check ? color1.GetValue() : color2.GetValue();
    }
  }

  return Texture(width, height, std::move(pixels));
}

Texture TextureStore::CreateSolid(int width, int height, Color color) {
  if (width <= 0 || height <= 0) {
    return Texture();
  }
  return Texture(width, height, color);
}
