#pragma once

#include <cmath>
#include "ViewSettings.h"

namespace render {
    class Projection {
        double center_;
        double half_fov_;
        double focal_;
        double horizon_;
        double eye_height_;
        const ViewSettings& settings_;

    public:
        Projection(int width, int height, double fov, double eye_height,
                   const ViewSettings& settings)
            : center_((width - 1) / 2.0), half_fov_(fov / 2.0),
              focal_(center_ / std::tan(half_fov_)), horizon_(height / 2.0),
              eye_height_(eye_height), settings_(settings) {}

        [[nodiscard]] double screenX(double slope) const {
            return center_ + (settings_.angle_projection
                ? center_ * std::atan(slope) / half_fov_
                : focal_ * slope);
        }

        [[nodiscard]] double slopeAt(double column) const {
            return settings_.angle_projection
                ? std::tan((column - center_) * half_fov_ / center_)
                : (column - center_) / focal_;
        }

        [[nodiscard]] double renderDepth(double forward_depth, double slope) const {
            return settings_.fish_eye ? forward_depth * std::hypot(1.0, slope) : forward_depth;
        }

        [[nodiscard]] double screenY(double height, double depth) const {
            return horizon_ - focal_ * (height - eye_height_) / depth;
        }

        [[nodiscard]] double horizon() const { return horizon_; }
    };
}
