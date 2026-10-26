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

TEST_F(RaycasterTest, ProjectsWallAroundScreenCenter) {
  RayHit hit;
  hit.hit = true;
  hit.depth = 2.0;
  hit.cell_x = 5;
  hit.cell_y = 2;
  hit.wall_id = 4;
  hit.side = WallSide::kX;
  const WallColumn column = raycaster_.ProjectWall(hit, 480);
  EXPECT_DOUBLE_EQ(column.projected_height, 240.0);
  EXPECT_EQ(column.draw_start, 120);
  EXPECT_EQ(column.draw_end, 360);
  EXPECT_TRUE(column.ray_hit.hit);
  EXPECT_DOUBLE_EQ(column.ray_hit.depth, 2.0);
  EXPECT_EQ(column.ray_hit.cell_x, 5);
  EXPECT_EQ(column.ray_hit.cell_y, 2);
  EXPECT_EQ(column.ray_hit.wall_id, 4);
  EXPECT_EQ(column.ray_hit.side, WallSide::kX);

  hit.depth = 4.0;
  const WallColumn farther = raycaster_.ProjectWall(hit, 480);
  EXPECT_DOUBLE_EQ(farther.projected_height, 120.0);
  EXPECT_EQ(farther.draw_start, 180);
  EXPECT_EQ(farther.draw_end, 300);
}

TEST_F(RaycasterTest, RoundsFractionalProjectionOutward) {
  RayHit hit;
  hit.hit = true;
  hit.depth = 2.0;
  const WallColumn column = raycaster_.ProjectWall(hit, 5);
  EXPECT_DOUBLE_EQ(column.projected_height, 2.5);
  EXPECT_EQ(column.draw_start, 1);
  EXPECT_EQ(column.draw_end, 4);
}

TEST_F(RaycasterTest, ClipsCloseWallWithoutChangingDepth) {
  RayHit hit;
  hit.hit = true;
  hit.depth = 0.25;
  const WallColumn column = raycaster_.ProjectWall(hit, 480);
  EXPECT_DOUBLE_EQ(column.projected_height, 1920.0);
  EXPECT_EQ(column.draw_start, 0);
  EXPECT_EQ(column.draw_end, 480);
  EXPECT_DOUBLE_EQ(column.ray_hit.depth, 0.25);
}

TEST_F(RaycasterTest, ProjectsZeroAndTinyDepthWithoutOverflow) {
  RayHit hit;
  hit.hit = true;
  const int height = std::numeric_limits<int>::max();
  for (const double depth : {0.0, 1e-300}) {
    hit.depth = depth;
    const WallColumn column = raycaster_.ProjectWall(hit, height);
    EXPECT_TRUE(std::isfinite(column.projected_height));
    EXPECT_EQ(column.draw_start, 0);
    EXPECT_EQ(column.draw_end, height);
    EXPECT_DOUBLE_EQ(column.ray_hit.depth, depth);
  }
}

TEST_F(RaycasterTest, LeavesMissAndInvalidProjectionEmpty) {
  const WallColumn miss = raycaster_.ProjectWall({}, 480);
  EXPECT_FALSE(miss.ray_hit.hit);
  EXPECT_DOUBLE_EQ(miss.projected_height, 0.0);
  EXPECT_EQ(miss.draw_start, miss.draw_end);

  RayHit hit;
  hit.hit = true;
  for (const double depth : {-1.0, std::numeric_limits<double>::infinity(),
                             std::numeric_limits<double>::quiet_NaN()}) {
    hit.depth = depth;
    const WallColumn column = raycaster_.ProjectWall(hit, 480);
    EXPECT_DOUBLE_EQ(column.projected_height, 0.0);
    EXPECT_EQ(column.draw_start, column.draw_end);
  }
  hit.depth = 2.0;
  for (const int height : {0, -1}) {
    const WallColumn column = raycaster_.ProjectWall(hit, height);
    EXPECT_DOUBLE_EQ(column.projected_height, 0.0);
    EXPECT_EQ(column.draw_start, column.draw_end);
  }
}

TEST_F(RaycasterTest, CastsScreenColumnsWithoutFisheye) {
  // Ровная стена перед камерой должна иметь одну высоту по всему экрану.
  const auto columns = raycaster_.CastColumns(map_, {3.5f, 3.5f}, {1.0f, 0.0f},
                                              {0.0f, 0.5f}, 4, 300);
  ASSERT_EQ(columns.size(), 4u);
  const int expected_y[] = {2, 3, 3, 3};
  for (int x = 0; x < 4; ++x) {
    SCOPED_TRACE(x);
    const WallColumn& column = columns[x];
    ASSERT_TRUE(column.ray_hit.hit);
    EXPECT_EQ(column.ray_hit.cell_x, 5);
    EXPECT_EQ(column.ray_hit.cell_y, expected_y[x]);
    EXPECT_EQ(column.ray_hit.wall_id, 4);
    EXPECT_EQ(column.ray_hit.side, WallSide::kX);
    EXPECT_DOUBLE_EQ(column.ray_hit.depth, 1.5);
    EXPECT_DOUBLE_EQ(column.projected_height, 200.0);
    EXPECT_EQ(column.draw_start, 50);
    EXPECT_EQ(column.draw_end, 250);
  }
}

TEST_F(RaycasterTest, KeepsScreenOrientationWhenLookingDown) {
  // При взгляде вниз правая сторона экрана направлена влево по карте.
  const auto columns = raycaster_.CastColumns(map_, {3.5f, 3.5f}, {0.0f, 1.0f},
                                              {-0.5f, 0.0f}, 4, 300);
  ASSERT_EQ(columns.size(), 4u);
  const int expected_x[] = {4, 3, 3, 3};
  for (int x = 0; x < 4; ++x) {
    SCOPED_TRACE(x);
    const WallColumn& column = columns[x];
    ASSERT_TRUE(column.ray_hit.hit);
    EXPECT_EQ(column.ray_hit.cell_x, expected_x[x]);
    EXPECT_EQ(column.ray_hit.cell_y, 5);
    EXPECT_EQ(column.ray_hit.wall_id, 3);
    EXPECT_EQ(column.ray_hit.side, WallSide::kY);
    EXPECT_DOUBLE_EQ(column.ray_hit.depth, 1.5);
    EXPECT_DOUBLE_EQ(column.projected_height, 200.0);
    EXPECT_EQ(column.draw_start, 50);
    EXPECT_EQ(column.draw_end, 250);
  }
}

TEST_F(RaycasterTest, KeepsMissesAtTheirScreenColumns) {
  map_.GetTile(5, 3).type_id = 0;
  const auto columns = raycaster_.CastColumns(map_, {3.5f, 3.5f}, {1.0f, 0.0f},
                                              {0.0f, 0.5f}, 4, 300);
  ASSERT_EQ(columns.size(), 4u);
  EXPECT_TRUE(columns[0].ray_hit.hit);
  // Наклонные лучи задевают соседние стены по краям проёма.
  EXPECT_TRUE(columns[1].ray_hit.hit);
  EXPECT_DOUBLE_EQ(columns[1].ray_hit.depth, 2.0);
  EXPECT_FALSE(columns[2].ray_hit.hit);
  EXPECT_DOUBLE_EQ(columns[2].projected_height, 0.0);
  EXPECT_EQ(columns[2].draw_start, columns[2].draw_end);
  EXPECT_TRUE(columns[3].ray_hit.hit);
  EXPECT_DOUBLE_EQ(columns[3].ray_hit.depth, 2.0);
}

TEST_F(RaycasterTest, RejectsInvalidScreenDimensions) {
  for (const Vec2i size :
       {Vec2i{0, 300}, Vec2i{-1, 300}, Vec2i{4, 0}, Vec2i{4, -1}}) {
    EXPECT_TRUE(raycaster_
                    .CastColumns(map_, {3.5f, 3.5f}, {1.0f, 0.0f}, {0.0f, 0.5f},
                                 size.x(), size.y())
                    .empty());
  }
}

TEST_F(RaycasterTest, RejectsInvalidCamera) {
  const float nan = std::numeric_limits<float>::quiet_NaN();
  const float infinity = std::numeric_limits<float>::infinity();
  const Vec2f invalid_directions[] = {
      {0.0f, 0.0f}, {2.0f, 0.0f}, {nan, 0.0f}, {infinity, 0.0f}};
  for (const Vec2f direction : invalid_directions) {
    EXPECT_TRUE(
        raycaster_
            .CastColumns(map_, {3.5f, 3.5f}, direction, {0.0f, 0.5f}, 4, 300)
            .empty());
  }
  const Vec2f invalid_planes[] = {
      {0.0f, 0.0f}, {0.5f, 0.5f}, {0.0f, -0.5f}, {0.0f, nan}, {0.0f, infinity}};
  for (const Vec2f plane : invalid_planes) {
    EXPECT_TRUE(
        raycaster_.CastColumns(map_, {3.5f, 3.5f}, {1.0f, 0.0f}, plane, 4, 300)
            .empty());
  }
}

TEST_F(RaycasterTest, KeepsInvalidPositionAsMisses) {
  const auto columns = raycaster_.CastColumns(map_, {-0.5f, 3.5f}, {1.0f, 0.0f},
                                              {0.0f, 0.5f}, 4, 300);
  ASSERT_EQ(columns.size(), 4u);
  for (const WallColumn& column : columns) {
    EXPECT_FALSE(column.ray_hit.hit);
    EXPECT_EQ(column.draw_start, column.draw_end);
  }
}

TEST_F(RaycasterTest, AcceptsRotatedCamera) {
  const auto columns = raycaster_.CastColumns(map_, {3.5f, 3.5f}, {0.6f, 0.8f},
                                              {-0.4f, 0.3f}, 2, 300);
  ASSERT_EQ(columns.size(), 2u);
  ASSERT_TRUE(columns[0].ray_hit.hit);
  EXPECT_EQ(columns[0].ray_hit.cell_x, 5);
  EXPECT_EQ(columns[0].ray_hit.cell_y, 4);
  EXPECT_NEAR(columns[0].ray_hit.depth, 1.5, 1e-6);
  ASSERT_TRUE(columns[1].ray_hit.hit);
  EXPECT_EQ(columns[1].ray_hit.cell_x, 4);
  EXPECT_EQ(columns[1].ray_hit.cell_y, 5);
  EXPECT_NEAR(columns[1].ray_hit.depth, 1.875, 1e-6);
  EXPECT_NEAR(columns[1].projected_height, 160.0, 1e-4);
}

}  // namespace
