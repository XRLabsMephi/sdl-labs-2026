#include "core/application.h"

#include <SDL3/SDL.h>

#include <stdexcept>

void SdlWindowDeleter::operator()(SDL_Window* window) const noexcept {
  if (window != nullptr) {
    SDL_DestroyWindow(window);
  }
}

void SdlRendererDeleter::operator()(SDL_Renderer* renderer) const noexcept {
  if (renderer != nullptr) {
    SDL_DestroyRenderer(renderer);
  }
}

Application::Application(Config config)
    : config_(config),
      is_running_(true),
      map_(Map::FromFile(config_.map_file)),
      frame_buffer_(config_.width, config_.height) {
  InitSdl();
  InitPlayerSpawn();
}

Application::~Application() {
  presenter_.reset();
  renderer_.reset();
  window_.reset();
  SDL_Quit();
}

void Application::InitSdl() {
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    throw std::runtime_error(std::string("Failed to initialize SDL: ") +
                             SDL_GetError());
  }

  SDL_WindowFlags window_flags = 0;
  if (config_.screen_mode == Config::ScreenMode::Fullscreen) {
    window_flags |= SDL_WINDOW_FULLSCREEN;
  }

  SDL_Window* raw_window = SDL_CreateWindow(
      "Software Graphics Labs - Wolfenstein 3D", config_.width, config_.height,
      window_flags);
  if (raw_window == nullptr) {
    SDL_Quit();
    throw std::runtime_error(std::string("Failed to create SDL Window: ") +
                             SDL_GetError());
  }
  window_.reset(raw_window);

  SDL_Renderer* raw_renderer = SDL_CreateRenderer(window_.get(), nullptr);
  if (raw_renderer == nullptr) {
    window_.reset();
    SDL_Quit();
    throw std::runtime_error(std::string("Failed to create SDL Renderer: ") +
                             SDL_GetError());
  }
  renderer_.reset(raw_renderer);

  presenter_ = std::make_unique<SdlPresenter>(renderer_.get(), config_.width,
                                              config_.height);
}

void Application::InitPlayerSpawn() {
  for (int y = 0; y < map_.GetHeight(); ++y) {
    for (int x = 0; x < map_.GetWidth(); ++x) {
      if (!map_.IsWall(x, y)) {
        player_.position = Vec2f{x + 0.5f, y + 0.5f};
        player_.SetDirection(Vec2f{1.0f, 0.0f});
        return;
      }
    }
  }

  player_.position = Vec2f{1.5f, 1.5f};
  player_.SetDirection(Vec2f{1.0f, 0.0f});
}

InputState Application::PollInput() {
  InputState input;
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_EVENT_QUIT) {
      input.quit = true;
      is_running_ = false;
    } else if (event.type == SDL_EVENT_KEY_DOWN) {
      if (event.key.key == SDLK_ESCAPE) {
        input.quit = true;
        is_running_ = false;
      }
    }
  }

  const bool* keys = SDL_GetKeyboardState(nullptr);
  if (keys != nullptr) {
    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) {
      input.move_forward = true;
    }
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
      input.move_backward = true;
    }
    if (keys[SDL_SCANCODE_A]) {
      input.strafe_left = true;
    }
    if (keys[SDL_SCANCODE_D]) {
      input.strafe_right = true;
    }
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_Q]) {
      input.rotate_left = true;
    }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_E]) {
      input.rotate_right = true;
    }
  }

  return input;
}

Color Application::ResolveWallColor(const RayHit& hit) const {
  Color wall_color = Color::kWhite;
  if (map_.InBounds(hit.cell_x, hit.cell_y)) {
    wall_color = map_.GetTileType(hit.cell_x, hit.cell_y).color;
  }

  if (hit.side == WallSide::kY) {
    const uint32_t val = wall_color.GetValue();
    const uint8_t r = static_cast<uint8_t>(((val >> 24) & 0xFF) * 0.75f);
    const uint8_t g = static_cast<uint8_t>(((val >> 16) & 0xFF) * 0.75f);
    const uint8_t b = static_cast<uint8_t>(((val >> 8) & 0xFF) * 0.75f);
    const uint8_t a = static_cast<uint8_t>(val & 0xFF);
    return Color(r, g, b, a);
  }

  return wall_color;
}

void Application::Run() {
  // Constant fixed-step update every frame (no delta-time calculation)
  constexpr float kFixedDt = 0.016f;

  while (is_running_) {
    const InputState input = PollInput();
    if (input.quit || !is_running_) {
      break;
    }

    controller_.Update(player_, map_, input, kFixedDt);

    // Ceiling (dark slate) and floor (dark gray)
    frame_buffer_.Clear(Color(50, 50, 60), Color(30, 30, 30));

    const auto columns = raycaster_.CastColumns(
        map_, player_.position, player_.direction, player_.camera_plane,
        config_.width, config_.height);

    for (int x = 0; x < config_.width; ++x) {
      const auto& col = columns[x];
      if (col.ray_hit.hit && col.draw_start < col.draw_end) {
        frame_buffer_.DrawVerticalLine(x, col.draw_start, col.draw_end,
                                       ResolveWallColor(col.ray_hit));
      }
    }

    presenter_->Present(frame_buffer_);
    SDL_Delay(16);
  }
}
