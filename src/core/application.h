#ifndef CORE_APPLICATION_H_
#define CORE_APPLICATION_H_

#include <memory>
#include <string>
#include <vector>

#include "core/color.h"
#include "core/config.h"
#include "effects/fog.h"
#include "input/input_state.h"
#include "input/player_controller.h"
#include "render/frame_buffer.h"
#include "render/sdl_presenter.h"
#include "render/texture_store.h"
#include "sprite/sprite_renderer.h"
#include "wolf/raycaster.h"
#include "world/map.h"
#include "world/player.h"

struct SDL_Window;
struct SDL_Renderer;

struct SdlWindowDeleter {
  void operator()(SDL_Window* window) const noexcept;
};

struct SdlRendererDeleter {
  void operator()(SDL_Renderer* renderer) const noexcept;
};

class Application {
 public:
  explicit Application(Config config);
  ~Application();

  Application(const Application&) = delete;
  Application& operator=(const Application&) = delete;

  void Run();
  // Builds one CPU frame; Run presents it after input and simulation updates.
  void RenderFrame();
  [[nodiscard]] const FrameBuffer& GetFrameBuffer() const { return frame_buffer_; }
  [[nodiscard]] const Player& GetPlayer() const { return player_; }

  [[nodiscard]] const std::vector<double>& GetDepthBuffer() const noexcept {
    return depth_buffer_;
  }

  [[nodiscard]] const Map& GetMap() const noexcept {
    return map_;
  }

  [[nodiscard]] TextureStore& GetTextureStore() noexcept {
    return texture_store_;
  }

  [[nodiscard]] const TextureStore& GetTextureStore() const noexcept {
    return texture_store_;
  }

 private:
  using UniqueWindow = std::unique_ptr<SDL_Window, SdlWindowDeleter>;
  using UniqueRenderer = std::unique_ptr<SDL_Renderer, SdlRendererDeleter>;

  void InitSdl();
  void InitPlayerSpawn();
  void LoadSceneTextures();
  InputState PollInput();
  Color ResolveWallColor(const RayHit& hit) const;
  void DrawWallColumn(int x, const WallColumn& col);

  Config config_;
  bool is_running_ = true;

  UniqueWindow window_;
  UniqueRenderer renderer_;

  Map map_;
  Player player_;
  PlayerController controller_;
  Raycaster raycaster_;
  FrameBuffer frame_buffer_;
  TextureStore texture_store_;
  std::vector<Texture> sprite_textures_;
  SpriteRenderer sprite_renderer_;
  Fog fog_;
  std::vector<double> depth_buffer_;
  std::unique_ptr<SdlPresenter> presenter_;
};

#endif  // CORE_APPLICATION_H_
