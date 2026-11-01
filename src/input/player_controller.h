#ifndef INPUT_PLAYER_CONTROLLER_H_
#define INPUT_PLAYER_CONTROLLER_H_

#include "input/input_state.h"
#include "world/map.h"
#include "world/player.h"

class PlayerController {
 public:
  struct Config {
    float move_speed = 3.0f;
    float rotate_speed = 2.5f;
    float collision_radius = 0.2f;
  };

  PlayerController();
  explicit PlayerController(const Config& config);

  void Update(Player& player, const Map& map, const InputState& input,
              float dt) const;

  [[nodiscard]] bool CanMoveTo(const Map& map, const Vec2f& position,
                               float radius) const;

  [[nodiscard]] const Config& GetConfig() const { return config_; }
  void SetConfig(const Config& config) { config_ = config; }

 private:
  Config config_;
};

#endif  // INPUT_PLAYER_CONTROLLER_H_
