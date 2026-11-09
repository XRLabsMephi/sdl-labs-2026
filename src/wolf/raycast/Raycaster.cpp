#include "wolf/raycast/Raycaster.hpp"

#include <cmath>
#include <cstdint>
#include <algorithm>

#include "wolf/core/color.hpp"

namespace {
    constexpr double   kSideShadeFactor = 0.7;   // shade y-sides
    constexpr double   kEpsilon  = 1e-9;         // 0 division
    constexpr double   kHugeDist = 1e30;         // no hit
    constexpr uint32_t kMaxSteps = 1024;         // DDA limit

    // Colors
    core::ColorRGBA wallColor(uint32_t wallId, uint32_t side) {
        core::ColorRGBA color;

        switch (wallId) {
            case 1:  color = {110, 120, 145, 255}; break;
            case 2:  color = {190,  95,  75, 200}; break;
            case 3:  color = { 90, 160, 100, 40 }; break;
            default: color = {120, 120, 120, 255}; break;
        }

        if (side == 1) {
            color.r = static_cast<std::uint8_t>(color.r * kSideShadeFactor);
            color.g = static_cast<std::uint8_t>(color.g * kSideShadeFactor);
            color.b = static_cast<std::uint8_t>(color.b * kSideShadeFactor);
        }

        return color;
    }
}

namespace raycast {
    void Raycaster::castRay(const world::Player& player,
                            const world::Map&    map,
                            double               cameraX) noexcept {

        RayHit result{};
        rayHits_.clear();

        const core::Vec2 pos   = player.position();
        const core::Vec2 dir   = player.direction();
        const core::Vec2 plane = player.cameraPlane();

        // Resulting ray
        const core::Vec2 rayDir = dir + plane * cameraX;

        int32_t mapX = static_cast<int32_t>(std::floor(pos.x));
        int32_t mapY = static_cast<int32_t>(std::floor(pos.y));

        // Player is inside wall or outside of the map
        if (map.isSolid(mapX, mapY)) {
            result.hit    = true;
            result.depth  = kEpsilon;
            result.mapX   = mapX;
            result.mapY   = mapY;
            result.wallId = map.cellAt(mapX, mapY);

            rayHits_.push_back(result);
            return;
        }

        // where to move to get the first intersection
        int32_t sgnX = (rayDir.x > kEpsilon) ? 1 : ((rayDir.x < -kEpsilon) ? -1 : 0),
                sgnY = (rayDir.y > kEpsilon) ? 1 : ((rayDir.y < -kEpsilon) ? -1 : 0);


        if (sgnX == 0 && sgnY == 0) {
            result.depth = kHugeDist;

            rayHits_.push_back(result);
            return;
        }

        // slope
        double k    = (sgnX != 0) ? rayDir.y / rayDir.x : 0.0;
        double invK = (sgnY != 0) ? rayDir.x / rayDir.y : 0.0;

        core::Vec2 LastPoint = pos;

        double FirstX = mapX + ((rayDir.x > 0) ? 1.0 : 0.0), // first intersection with x
               FirstY = mapY + ((rayDir.y > 0) ? 1.0 : 0.0); // first intersection with y

        core::Vec2 PointX{FirstX, LastPoint.y + (FirstX - LastPoint.x) * k},    // on x intersection
                   PointY{LastPoint.x + (FirstY - LastPoint.y) * invK, FirstY}; // on y intersection

        double rayStregnth = 1;

        for (uint32_t i = 0; i < kMaxSteps; ++i) {
            if (sgnY == 0) {
                // horizontal ray: only x intersections happen
                LastPoint = PointX;

                mapX += sgnX;
                FirstX += sgnX;

                PointX = {FirstX, LastPoint.y + (FirstX - LastPoint.x) * k};

                result.side = 0;
            }
            else if (sgnX == 0) {
                // vertical ray: only y intersections happen
                LastPoint = PointY;

                mapY += sgnY;
                FirstY += sgnY;

                PointY = {LastPoint.x + (FirstY - LastPoint.y) * invK, FirstY};

                result.side = 1;
            }
            else {
                if (sgnX * (PointX.x - PointY.x) < 0) {
                    LastPoint = PointX;

                    mapX += sgnX;
                    FirstX += sgnX;

                    PointX = {FirstX, LastPoint.y + (FirstX - LastPoint.x) * k};

                    result.side = 0;
                }
                else {
                    LastPoint = PointY;
                    
                    mapY += sgnY;
                    FirstY += sgnY;

                    PointY = {LastPoint.x + (FirstY - LastPoint.y) * invK, FirstY};

                    result.side = 1;
                }
            }

            if (map.isSolid(mapX, mapY) || i == kMaxSteps - 1) {
                if (map.isSolid(mapX, mapY))
                    result.hit = true;

                result.mapX   = mapX;
                result.mapY   = mapY;
                result.wallId = map.cellAt(mapX, mapY);

                if (!result.hit) {
                    result.depth = kHugeDist;

                    rayHits_.push_back(result);
                    return;
                }

                // perpendicular depth: t where Ray == t * rayDir

                core::Vec2 Ray = LastPoint - pos;

                /*
                    Ray == rayDir * t == [dir + plane * cameraX] * t
                    depth = (Ray, dir) / |dir| == t * ([dir + plane * cameraX], dir) == t * [1 + (plane * cameraX, dir)] == t
                    depth = t = Ray / rayDir
                */

                if (result.side == 0) { // wall is parallel to y, LastPoint.x = int
                    result.depth = Ray.x / rayDir.x;
                    result.wallU = LastPoint.y - (double)result.mapY;
                }
                else { // wall is parallel to x, LastPoint.y = int
                    result.depth = Ray.y / rayDir.y;
                    result.wallU = LastPoint.x - (double)result.mapX;
                }

                if (Ray.x > 0.0 && result.side == 0 || Ray.y < 0.0 && result.side == 1) {
                    result.wallU = 1.0 - result.wallU; // if against axis -> invert
                }

                if (result.depth < kEpsilon)
                    result.depth = kEpsilon;

                const uint32_t color = wallColor(result.wallId, result.side).pack();
                const double   alpha = color >> 24;

                rayStregnth *= (1 - alpha / 255.0);
                
                rayHits_.push_back(result);

                if (rayStregnth < 1e-3)
                    break;
            }
        }
    }

    void Raycaster::render(const world::Player&         player,
                           const world::Map&            map,
                           const texture::TextureStore& textures,
                           const effects::Fog&          fog,
                           render::Framebuffer&         framebuffer) noexcept {

        const uint32_t w = framebuffer.width();
        const uint32_t h = framebuffer.height();
        
        // on framebuffer width change
        if (depthBuffer_.size() != w) {
            depthBuffer_.resize(w);
        }

        const uint32_t mid       = h / 2;           // screen center, no division
        const double   invW      = 2.0 / w;         // precompute 1/w once
        const double   maxHeight = h * 2.0;         // clamp before uint32_t cast (avoid UB)

        // raw pixel buffer — no bounds check per pixel
        uint32_t* pixels = framebuffer.color();

        FrameCtx f = {
            pixels,
            w,
            h,
            fog
        };

        for (uint32_t x = 0; x < w; ++x) {
            // cameraX [-1, +1]: left to right
            const double cameraX = invW * x - 1.0;

            castRay(player, map, cameraX);

            // fill depth buffer
            depthBuffer_[x] = kHugeDist;
        
            if (!rayHits_.empty() && rayHits_[0].hit) {
                depthBuffer_[x] = rayHits_[0].depth; // closest wall
            } 

            for (int32_t i = rayHits_.size() - 1; i >= 0; --i) {
                const RayHit& rh = rayHits_[i];

                if (!rh.hit) // inf
                    continue;

                // less depth - bigger wall
                double lineHeight = static_cast<double>(h) / rh.depth;

                // clamp BEFORE uint32_t cast — otherwise lineHeight can overflow uint32_t
                if (lineHeight > maxHeight)
                    lineHeight = maxHeight;

                const uint32_t drawStart = static_cast<uint32_t>(std::max(mid - lineHeight * 0.5, 0.0));       // >= 0
                const uint32_t drawEnd   = static_cast<uint32_t>(std::min(mid + lineHeight * 0.5, (double)h)); // <= h

                const texture::Texture* tex = textures.get(rh.wallId);

                ColumnCtx c = {
                    x,
                    drawStart,
                    drawEnd,
                    lineHeight,
                };

                if (tex == nullptr || tex->width <= 0 || tex->height <= 0) {
                    drawSolidColumn(f, c, rh);
                }
                else {
                    drawTexturedColumn(f, c, rh, *tex);
                }
            }
        }
    }

    void Raycaster::drawTexturedColumn(const FrameCtx&  f,
                                       const ColumnCtx& c,
                                       const RayHit&    rh,
                                       const texture::Texture& tex) noexcept {

        int32_t texX = static_cast<int32_t>(rh.wallU * static_cast<double>(tex.width));
        
        if (texX < 0)
            texX = 0;

        if (texX >= tex.width)
            texX = tex.width - 1;

        const double step   = static_cast<double>(tex.height) / c.lineHeight;
        double       texPos = std::max(1.0 - rh.depth, 0.0) * static_cast<double>(tex.height) / 2.0;

        uint32_t* const pixels = f.pixels;
        const uint32_t  w      = f.w;
        
        for (uint32_t y = c.drawStart; y < c.drawEnd; ++y) {
            int texY = static_cast<int>(texPos);

            if (texY < 0)           
                texY = 0;
            
            if (texY >= tex.height) 
                texY = tex.height - 1;

            texPos += step;

            const uint32_t texColor = tex.sample(texX, texY);

            uint8_t r = (texColor >>  0) & 0xFF;
            uint8_t g = (texColor >>  8) & 0xFF;
            uint8_t b = (texColor >> 16) & 0xFF;
            uint8_t a = (texColor >> 24) & 0xFF;

            // shade
            if (rh.side == 1) {
                r = static_cast<uint8_t>(r * kSideShadeFactor);
                g = static_cast<uint8_t>(g * kSideShadeFactor);
                b = static_cast<uint8_t>(b * kSideShadeFactor);
            }

            core::ColorRGBA color(r, g, b, a);

            // fog
            color = f.fog.apply(color, rh.depth);

            pixels[y * w + c.x] = color.pack();
        }             
    }

    void Raycaster::drawSolidColumn(const FrameCtx&  f,
                                    const ColumnCtx& c,
                                    const RayHit&    rh) noexcept {
        
        // pack ONCE per column, not per pixel
        const uint32_t color = wallColor(rh.wallId, rh.side).pack();

        uint8_t alpha = color >> 24;
        uint8_t rev_a = 0xFF - alpha;

        uint32_t remove_a = 0x00FFFFFF;

        // direct write: one store + one add per pixel
        for (uint32_t col_idx = c.x + c.drawStart * f.w; col_idx < c.drawEnd * f.w; col_idx += f.w) {
            uint32_t prev_color = f.pixels[col_idx] & remove_a;
            uint32_t new_color  = color             & remove_a;

            // uint32_t result_color = (prev_color * rev_a + new_color * alpha) / 255 + fill_a;

            uint8_t r = (((prev_color >> 0 ) & 0xFF) * rev_a + ((new_color >> 0 ) & 0xFF) * alpha) / 255u;
            uint8_t g = (((prev_color >> 8 ) & 0xFF) * rev_a + ((new_color >> 8 ) & 0xFF) * alpha) / 255u;
            uint8_t b = (((prev_color >> 16) & 0xFF) * rev_a + ((new_color >> 16) & 0xFF) * alpha) / 255u;
            uint8_t a = 0xFF;

            core::ColorRGBA col{r, g, b, a};

            //apply fog
            col = f.fog.apply(col, rh.depth);

            f.pixels[col_idx] = col.pack();
        }
    }
}