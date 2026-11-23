#include "wolf/render/framebuffer.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace render {
    Framebuffer::Framebuffer(int width, int height) : width_(width), height_(height) {
        if (width <= 0 || height <= 0) {
            throw std::invalid_argument("Framebuffer dimensions must be positive");
        }

        const auto size = static_cast<size_t>(width_) * static_cast<size_t>(height_);
        data_.assign(size, core::ColorRGBA{}.pack());
        depth_.assign(size, std::numeric_limits<float>::infinity());
        transparentHeads_.assign(size, kNoFragment);
    }

    void Framebuffer::clear(core::ColorRGBA color) {
        std::fill(data_.begin(), data_.end(), color.pack());
        std::fill(depth_.begin(), depth_.end(), std::numeric_limits<float>::infinity());
        std::fill(transparentHeads_.begin(), transparentHeads_.end(), kNoFragment);
        transparentFragments_.clear();
    }

    int Framebuffer::width() const noexcept {
        return width_;
    }

    int Framebuffer::height() const noexcept {
        return height_;
    }

    uint32_t* Framebuffer::color() noexcept {
        return data_.data();
    }

    const uint32_t* Framebuffer::color() const noexcept {
        return data_.data();
    }

    float* Framebuffer::depth() noexcept {
        return depth_.data();
    }

    const float* Framebuffer::depth() const noexcept {
        return depth_.data();
    }

    void Framebuffer::setDepth(int x, int y, float depth) noexcept {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) return;
        depth_[width_ * y + x] = depth;
    }

    float Framebuffer::getDepth(int x, int y) const noexcept {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) return 0;
        return depth_[width_ * y + x];
    }

    void Framebuffer::setPixel(int x, int y, core::ColorRGBA color) noexcept {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) return;
        data_[width_ * y + x] = color.pack();
    }

    uint32_t Framebuffer::getPixel(int x, int y) const noexcept {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) return 0;
        return data_[width_ * y + x];
    }

    void Framebuffer::drawFragment(int x, int y, core::ColorRGBA color, double depth) {
        if (x < 0 || x >= width_ || y < 0 || y >= height_ || color.a == 0 ||
            !std::isfinite(depth) || depth <= 0.0) return;
        const auto index = static_cast<std::size_t>(y) * width_ + x;
        if (!(depth < depth_[index])) return;
        if (color.a == 255) {
            data_[index] = color.pack();
            depth_[index] = static_cast<float>(depth);
        } else {
            if (transparentFragments_.size() >= kNoFragment) {
                throw std::length_error("Too many transparent fragments in one frame");
            }
            const auto head = static_cast<uint32_t>(transparentFragments_.size());
            transparentFragments_.push_back({depth, color.pack(), transparentHeads_[index]});
            transparentHeads_[index] = head;
        }
    }

    const uint32_t* Framebuffer::transparentHeads() const noexcept {
        return transparentHeads_.data();
    }

    std::span<const Framebuffer::TransparentFragment> Framebuffer::transparentFragments() const noexcept {
        return transparentFragments_;
    }
}
