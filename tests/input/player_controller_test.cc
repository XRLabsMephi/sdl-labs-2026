#include "input/player_controller.h"

#include <gtest/gtest.h>

#include <cmath>
#include <string>

#include "world/map.h"
#include "world/player.h"

namespace {

class PlayerControllerTest : public ::testing::Test {
 protected:
  Map map_ = Map::FromFile(std::string(TEST_DATA_DIR) + "/valid_ascii.map");
  PlayerController controller_;
};

TEST_F(PlayerControllerTest, PlayerDefaultInitialization) {
  Player player;
  EXPECT_EQ(player.position, (Vec2f{0.0f, 0.0f}));
  EXPECT_NEAR(player.direction.Length(), 1.0f, 1e-5f);
  EXPECT_NEAR(player.direction.Dot(player.camera_plane), 0.0f, 1e-5f);
  EXPECT_GT(player.direction.Cross(player.camera_plane), 0.0f);
}

TEST_F(PlayerControllerTest, PlayerRotationMaintainsOrthogonality) {
  Player player({1.5f, 1.5f}, {1.0f, 0.0f}, 0.66f);
  const float half_pi = static_cast<float>(M_PI / 2.0);

  // Rotate 90 degrees right (towards +Y)
  player.Rotate(half_pi);
  EXPECT_NEAR(player.direction.x(), 0.0f, 1e-5f);
  EXPECT_NEAR(player.direction.y(), 1.0f, 1e-5f);
  EXPECT_NEAR(player.camera_plane.x(), -0.66f, 1e-5f);
  EXPECT_NEAR(player.camera_plane.y(), 0.0f, 1e-5f);
  EXPECT_NEAR(player.direction.Dot(player.camera_plane), 0.0f, 1e-5f);

  // Repeated rotations don't degrade normalization or orthogonality
  for (int i = 0; i < 72; ++i) {
    player.Rotate(static_cast<float>(M_PI / 36.0));
    EXPECT_NEAR(player.direction.Length(), 1.0f, 1e-4f);
    EXPECT_NEAR(player.direction.Dot(player.camera_plane), 0.0f, 1e-4f);
    EXPECT_GT(player.direction.Cross(player.camera_plane), 0.0f);
  }
}

TEST_F(PlayerControllerTest, MoveInOpenSpace) {
  Player player({1.5f, 1.5f}, {1.0f, 0.0f});
  InputState input;
  input.move_forward = true;

  controller_.Update(player, map_, input, 0.1f);
  // Default move_speed is 3.0, dt = 0.1 -> delta = 0.3 along +X
  EXPECT_NEAR(player.position.x(), 1.8f, 1e-5f);
  EXPECT_NEAR(player.position.y(), 1.5f, 1e-5f);
}

TEST_F(PlayerControllerTest, StrafeMovement) {
  Player player({1.5f, 1.5f}, {1.0f, 0.0f});
  InputState input;
  input.strafe_right = true;

  controller_.Update(player, map_, input, 0.1f);
  // Strafe right is along +camera_plane normalized (+Y)
  EXPECT_NEAR(player.position.x(), 1.5f, 1e-5f);
  EXPECT_NEAR(player.position.y(), 1.8f, 1e-5f);
}

TEST_F(PlayerControllerTest, CollisionStopsDirectMovement) {
  // Cell (0, 1) is a wall. Player starts at (1.25, 1.5) facing left (-1, 0).
  Player player({1.25f, 1.5f}, {-1.0f, 0.0f});
  InputState input;
  input.move_forward = true;

  // Collision radius is 0.2. Moving towards x=1.0 is blocked when x - radius < 1.0
  controller_.Update(player, map_, input, 0.5f);
  // Should not penetrate into wall (x must stay >= 1.2)
  EXPECT_GE(player.position.x(), 1.2f);
  EXPECT_EQ(player.position.y(), 1.5f);
}

TEST_F(PlayerControllerTest, SlidingAlongVerticalWall) {
  // Player at (1.25, 1.5) near wall at x <= 1.0.
  // Input: forward (-1, 0) and strafe right (0, 1), i.e. diagonal into the wall.
  Player player({1.25f, 1.5f}, {-1.0f, 0.0f});
  InputState input;
  input.move_forward = true;
  input.strafe_right = true;

  controller_.Update(player, map_, input, 0.1f);
  // When facing left (-X), right is up (-Y in screen/grid coordinates).
  // Movement in X is blocked by the left wall, but player slides along Y!
  EXPECT_EQ(player.position.x(), 1.25f);
  EXPECT_LT(player.position.y(), 1.5f);
}

TEST_F(PlayerControllerTest, SlidingAlongHorizontalWall) {
  // Cell (1, 0) is a wall. Player at (1.5, 1.25) facing up (0, -1).
  Player player({1.5f, 1.25f}, {0.0f, -1.0f});
  InputState input;
  input.move_forward = true;
  input.strafe_right = true;  // strafe right when facing up is +X

  controller_.Update(player, map_, input, 0.1f);
  // Movement in Y is blocked by top wall, but movement in X slides freely!
  EXPECT_GT(player.position.x(), 1.5f);
  EXPECT_EQ(player.position.y(), 1.25f);
}

TEST_F(PlayerControllerTest, ZeroDtOrNoInputDoesNotMove) {
  Player player({1.5f, 1.5f}, {1.0f, 0.0f});
  InputState input;
  input.move_forward = true;

  controller_.Update(player, map_, input, 0.0f);
  EXPECT_EQ(player.position.x(), 1.5f);
  EXPECT_EQ(player.position.y(), 1.5f);

  input.move_forward = false;
  controller_.Update(player, map_, input, 0.5f);
  EXPECT_EQ(player.position.x(), 1.5f);
  EXPECT_EQ(player.position.y(), 1.5f);
}

}  // namespace
