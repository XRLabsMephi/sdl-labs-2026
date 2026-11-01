#ifndef CORE_APPLICATION_H_
#define CORE_APPLICATION_H_

#include <memory>
#include <string>

#include "core/color.h"
#include "core/config.h"
#include "input/input_state.h"
#include "input/player_controller.h"
#include "render/frame_buffer.h"
#include "render/sdl_presenter.h"
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

 private:
  using UniqueWindow = std::unique_ptr<SDL_Window, SdlWindowDeleter>;
  using UniqueRenderer = std::unique_ptr<SDL_Renderer, SdlRendererDeleter>;

  void InitSdl();
  void InitPlayerSpawn();
  InputState PollInput();
  Color ResolveWallColor(const RayHit& hit) const;

  Config config_;
  bool is_running_ = true;

  UniqueWindow window_;
  UniqueRenderer renderer_;

  Map map_;
  Player player_;
  PlayerController controller_;
  Raycaster raycaster_;
  FrameBuffer frame_buffer_;
  std::unique_ptr<SdlPresenter> presenter_;
};

#endif  // CORE_APPLICATION_H_
