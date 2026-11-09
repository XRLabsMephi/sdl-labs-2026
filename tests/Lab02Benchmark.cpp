#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "support/Lab02Scene.hpp"
#include "wolf/effects/Fog.hpp"
#include "wolf/raycast/Raycaster.hpp"
#include "wolf/render/SdlPresenter.hpp"
#include "wolf/sprite/SpriteRenderer.hpp"

namespace {
    using Clock = std::chrono::steady_clock;

    struct Options {
        int width = 960;
        int height = 720;
        int frames = 180;
        int warmup = 30;
        std::string scene = "all";
        bool present = false;
    };

    Options parse(int argc, char** argv) {
        Options options;
        for (int i = 1; i < argc; ++i) {
            const std::string_view flag = argv[i];
            if (flag == "--present") { options.present = true; continue; }
            if (++i == argc) throw std::invalid_argument("Missing value for " + std::string(flag));
            if (flag == "--scene") { options.scene = argv[i]; continue; }
            const std::string_view text = argv[i];
            int value = 0;
            const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
            if (result.ec != std::errc{} || result.ptr != text.data() + text.size() || value < 0) {
                throw std::invalid_argument("Expected a nonnegative integer for " + std::string(flag));
            }
            if (flag == "--width" && value > 0 && value <= 8192) options.width = value;
            else if (flag == "--height" && value > 0 && value <= 8192) options.height = value;
            else if (flag == "--frames" && value > 0) options.frames = value;
            else if (flag == "--warmup") options.warmup = value;
            else throw std::invalid_argument("Invalid option: " + std::string(flag));
        }
        return options;
    }

    double ms(Clock::time_point start, Clock::time_point end) {
        return std::chrono::duration<double, std::milli>(end - start).count();
    }

    double percentile(std::vector<double> values, double fraction) {
        std::sort(values.begin(), values.end());
        return values[static_cast<std::size_t>((values.size() - 1) * fraction)];
    }

    void configure(lab02::DemoScene& scene, const std::string& kind) {
        if (kind == "demo") return;
        if (kind == "application") {
            scene.map = world::Map::defaultMap();
            scene.player = world::Player({2.5, 12.5}, {1.0, -1.0}, 80.0 / 57.2958);
            scene.sprites.clear();
        } else if (kind == "opaque") {
            scene.textures.load(3, std::filesystem::path(WOLF_ASSETS_DIR) / "textures/backrooms/glass.png");
        } else if (kind == "near_glass") {
            scene.player.setPosition({6.8, 7.5});
        } else if (kind == "eight_glass") {
            std::vector<std::vector<int>> cells(15, std::vector<int>(24, 0));
            for (int y = 0; y < 15; ++y) {
                for (int x = 0; x < 24; ++x) {
                    if (y == 0 || y == 14 || x == 0 || x == 23) cells[y][x] = 1;
                    else if (x >= 4 && x <= 18 && x % 2 == 0) cells[y][x] = 3;
                }
            }
            scene.map = world::Map(std::move(cells));
            scene.sprites = {{{3.0, 8.0}, 101}, {{5.0, 7.0}, 102, 0.65}, {{21.0, 7.5}, 103}};
        } else {
            throw std::invalid_argument("Unknown scene: " + kind);
        }
    }

    void benchmark(const Options& options, const std::string& kind, render::SdlPresenter* presenter) {
        // PNG decoding and resource setup are outside the frame timing.
        lab02::DemoScene scene;
        configure(scene, kind);
        render::Framebuffer frame(options.width, options.height);
        raycast::Raycaster walls;
        sprite::SpriteRenderer sprites;
        effects::Fog fog(3.0, 14.0, {160, 153, 107});
        std::array<double, 6> sum{};
        std::vector<double> totals;
        totals.reserve(options.frames);
        std::size_t peakFragments = 0;
        std::size_t maxLayers = 0;
        std::uint64_t checksum = 0;
        for (int iteration = -options.warmup; iteration < options.frames; ++iteration) {
            const auto start = Clock::now();
            frame.clear({145, 138, 101});
            std::fill(frame.color() + options.width * (options.height / 2),
                      frame.color() + options.width * options.height, core::ColorRGBA(83, 77, 47).pack());
            const auto cleared = Clock::now();
            walls.render(scene.player, scene.map, scene.textures, frame);
            const auto cast = Clock::now();
            sprites.render(scene.player, scene.sprites, scene.textures, walls.depthBuffer(), frame);
            const auto projected = Clock::now();
            fog.apply(frame);
            const auto composed = Clock::now();
            if (presenter) presenter->present(frame);
            const auto done = Clock::now();
            if (iteration < 0) continue;
            sum[0] += ms(start, cleared);
            sum[1] += ms(cleared, cast);
            sum[2] += ms(cast, projected);
            sum[3] += ms(projected, composed);
            sum[4] += ms(composed, done);
            sum[5] += ms(start, done);
            totals.push_back(ms(start, done));
            // Diagnostics run after the timing boundary, once per scene.
            if (iteration == 0) {
                peakFragments = frame.transparentFragments().size();
                for (int i = 0; i < options.width * options.height; ++i) {
                    std::size_t count = 0;
                    auto head = frame.transparentHeads()[i];
                    const auto fragments = frame.transparentFragments();
                    while (head != render::Framebuffer::kNoFragment) { ++count; head = fragments[head].next; }
                    maxLayers = std::max(maxLayers, count);
                    checksum = checksum * 31 + frame.color()[i];
                }
            }
        }
        // Live storage estimate excludes vector spare capacity, textures, and SDL resources.
        const auto liveBytes = static_cast<std::size_t>(options.width) * options.height * 12 +
            peakFragments * sizeof(render::Framebuffer::TransparentFragment);
        std::cout << kind << ',' << options.width << ',' << options.height << ',' << options.frames << ','
                  << options.warmup << ',' << (presenter ? 1 : 0);
        for (double value : sum) std::cout << ',' << value / options.frames;
        std::cout << ',' << percentile(totals, 0.5) << ',' << percentile(totals, 0.95)
                  << ',' << 1000.0 * options.frames / sum[5] << ',' << peakFragments << ',' << maxLayers
                  << ',' << liveBytes / (1024.0 * 1024.0) << ',' << checksum << '\n';
    }
}

int main(int argc, char** argv) {
    try {
        const auto options = parse(argc, argv);
        std::unique_ptr<render::SdlPresenter> presenter;
        if (options.present) {
            presenter = std::make_unique<render::SdlPresenter>(options.width, options.height, "LR2 benchmark");
            auto* renderer = SDL_GetRenderer(presenter->window());
            int vsync = 0;
            SDL_GetRenderVSync(renderer, &vsync);
            std::cerr << "SDL video=" << SDL_GetCurrentVideoDriver()
                      << ", renderer=" << SDL_GetRendererName(renderer) << ", vsync=" << vsync << '\n';
        }
        std::cout << "scene,width,height,frames,warmup,present,clear_ms,walls_ms,sprites_ms,fog_ms,present_ms,mean_ms,p50_ms,p95_ms,fps,fragments,max_layers,live_frame_mib,checksum\n";
        std::cout << std::fixed << std::setprecision(4);
        if (options.scene == "all") {
            for (const std::string scene : {"application", "opaque", "demo", "near_glass", "eight_glass"}) {
                benchmark(options, scene, presenter.get());
            }
        } else {
            benchmark(options, options.scene, presenter.get());
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Benchmark failed: " << error.what() << '\n';
        return 1;
    }
}
