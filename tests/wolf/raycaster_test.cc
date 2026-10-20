#include "wolf/raycaster.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <string>

#include "world/map.h"

namespace {

class RaycasterTest : public ::testing::Test {
 protected:
  Map map_ = Map::FromFile(std::string(TEST_DATA_DIR) + "/valid_ascii.map");
  Raycaster raycaster_;
};

TEST_F(RaycasterTest, HitsRightWall) {
  const RayHit hit = raycaster_.CastRay(map_, {1.5f, 1.5f}, {1.0f, 0.0f});
  ASSERT_TRUE(hit.hit);
  EXPECT_EQ(hit.cell_x, 5);
  EXPECT_EQ(hit.cell_y, 1);
  EXPECT_EQ(hit.wall_id, 4);
  EXPECT_EQ(hit.side, WallSide::kX);
  EXPECT_DOUBLE_EQ(hit.depth, 3.5);
}

TEST_F(RaycasterTest, HitsLeftWall) {
  const RayHit hit = raycaster_.CastRay(map_, {1.5f, 1.5f}, {-1.0f, 0.0f});
  ASSERT_TRUE(hit.hit);
  EXPECT_EQ(hit.cell_x, 0);
  EXPECT_EQ(hit.cell_y, 1);
  EXPECT_EQ(hit.wall_id, 2);
  EXPECT_EQ(hit.side, WallSide::kX);
  EXPECT_DOUBLE_EQ(hit.depth, 0.5);
}

TEST_F(RaycasterTest, PositiveYGoesDown) {
  const RayHit hit = raycaster_.CastRay(map_, {1.5f, 1.5f}, {0.0f, 1.0f});
  ASSERT_TRUE(hit.hit);
  EXPECT_EQ(hit.cell_x, 1);
  EXPECT_EQ(hit.cell_y, 5);
  EXPECT_EQ(hit.wall_id, 3);
  EXPECT_EQ(hit.side, WallSide::kY);
  EXPECT_DOUBLE_EQ(hit.depth, 3.5);
}

TEST_F(RaycasterTest, NegativeYGoesUp) {
  const RayHit hit = raycaster_.CastRay(map_, {1.5f, 1.5f}, {0.0f, -1.0f});
  ASSERT_TRUE(hit.hit);
  EXPECT_EQ(hit.cell_x, 1);
  EXPECT_EQ(hit.cell_y, 0);
  EXPECT_EQ(hit.wall_id, 1);
  EXPECT_EQ(hit.side, WallSide::kY);
  EXPECT_DOUBLE_EQ(hit.depth, 0.5);
}

TEST_F(RaycasterTest, StopsAtFirstWall) {
  const RayHit hit = raycaster_.CastRay(map_, {1.5f, 2.5f}, {1.0f, 0.0f});
  ASSERT_TRUE(hit.hit);
  EXPECT_EQ(hit.cell_x, 2);
  EXPECT_EQ(hit.cell_y, 2);
  EXPECT_EQ(hit.wall_id, 1);
  EXPECT_EQ(hit.side, WallSide::kX);
  EXPECT_DOUBLE_EQ(hit.depth, 0.5);
}

TEST_F(RaycasterTest, KeepsCameraDepthForUnnormalizedRay) {
  // Направление камеры (1, 0), вектор её плоскости (0, 0.5), camera_x = 0.5.
  const RayHit hit = raycaster_.CastRay(map_, {1.5f, 1.5f}, {1.0f, 0.25f});
  ASSERT_TRUE(hit.hit);
  EXPECT_EQ(hit.cell_x, 5);
  EXPECT_EQ(hit.cell_y, 2);
  EXPECT_EQ(hit.side, WallSide::kX);
  EXPECT_DOUBLE_EQ(hit.depth, 3.5);
}

TEST_F(RaycasterTest, ChoosesYBoundaryAtCorner) {
  const RayHit hit = raycaster_.CastRay(map_, {4.5f, 4.5f}, {1.0f, 1.0f});
  ASSERT_TRUE(hit.hit);
  EXPECT_EQ(hit.cell_x, 4);
  EXPECT_EQ(hit.cell_y, 5);
  EXPECT_EQ(hit.wall_id, 3);
  EXPECT_EQ(hit.side, WallSide::kY);
  EXPECT_DOUBLE_EQ(hit.depth, 0.5);
}

TEST_F(RaycasterTest, HandlesParallelRayStartingOnGridLine) {
  const RayHit hit = raycaster_.CastRay(map_, {1.5f, 1.0f}, {1.0f, 0.0f});
  ASSERT_TRUE(hit.hit);
  EXPECT_EQ(hit.cell_x, 5);
  EXPECT_EQ(hit.cell_y, 1);
  EXPECT_DOUBLE_EQ(hit.depth, 3.5);
}

TEST_F(RaycasterTest, PreservesZeroDepthAtWallBoundary) {
  const RayHit hit = raycaster_.CastRay(map_, {1.0f, 1.5f}, {-1.0f, 0.0f});
  ASSERT_TRUE(hit.hit);
  EXPECT_EQ(hit.cell_x, 0);
  EXPECT_EQ(hit.side, WallSide::kX);
  EXPECT_DOUBLE_EQ(hit.depth, 0.0);
}

TEST_F(RaycasterTest, ReturnsMissWhenLeavingOpenMap) {
  map_.GetTile(5, 1).type_id = 0;
  const RayHit hit = raycaster_.CastRay(map_, {1.5f, 1.5f}, {1.0f, 0.0f});
  EXPECT_FALSE(hit.hit);
  EXPECT_TRUE(std::isinf(hit.depth));
  EXPECT_EQ(hit.cell_x, -1);
  EXPECT_EQ(hit.cell_y, -1);
  EXPECT_EQ(hit.wall_id, 0);
  EXPECT_EQ(hit.side, WallSide::kNone);
}

TEST_F(RaycasterTest, RejectsZeroDirection) {
  EXPECT_FALSE(raycaster_.CastRay(map_, {1.5f, 1.5f}, {0.0f, 0.0f}).hit);
}

TEST_F(RaycasterTest, RejectsStartingOutsideMapOrInsideWall) {
  EXPECT_FALSE(raycaster_.CastRay(map_, {-0.5f, 1.5f}, {1.0f, 0.0f}).hit);
  EXPECT_FALSE(raycaster_.CastRay(map_, {6.0f, 1.5f}, {-1.0f, 0.0f}).hit);
  EXPECT_FALSE(raycaster_.CastRay(map_, {2.5f, 2.5f}, {1.0f, 0.0f}).hit);
}

TEST_F(RaycasterTest, RejectsNonFiniteInput) {
  const float infinity = std::numeric_limits<float>::infinity();
  const float nan = std::numeric_limits<float>::quiet_NaN();
  EXPECT_FALSE(raycaster_.CastRay(map_, {infinity, 1.5f}, {1.0f, 0.0f}).hit);
  EXPECT_FALSE(raycaster_.CastRay(map_, {1.5f, nan}, {1.0f, 0.0f}).hit);
  EXPECT_FALSE(raycaster_.CastRay(map_, {1.5f, 1.5f}, {infinity, 0.0f}).hit);
  EXPECT_FALSE(raycaster_.CastRay(map_, {1.5f, 1.5f}, {0.0f, nan}).hit);
}

}  // namespace
