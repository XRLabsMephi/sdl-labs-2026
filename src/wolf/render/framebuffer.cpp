#include "wolf/render/framebuffer.hpp"
#include <algorithm>
#include <stdexcept>

namespace render {
    Framebuffer::Framebuffer(int width, int height) : width_(width), height_(height) {
        if (width <= 0 || height <= 0) {
            throw std::invalid_argument("Framebuffer dimensions must be positive");
        }

        const auto size = static_cast<size_t>(width_) * static_cast<size_t>(height_);
        data_.assign(size, core::ColorRGBA{}.pack());
        depth_.assign(size, 1.0f);
    }

    void Framebuffer::clear(core::ColorRGBA color) {
        std::fill(data_.begin(), data_.end(), color.pack());
        std::fill(depth_.begin(), depth_.end(), 1.0f);
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
}
