#include <vector>
#include <algorithm>

#include <core/Vec2.h>
#include <world/Sprite.h>
#include <render/FrameBuffer.h>
#include <render/TextureStorage.h>
#include <world/Player.h>
#include <world/Map.h>

#include "ViewSettings.h"

namespace render{


class SpriteRenderer
{

    using Vector2 = core::Vector2;

    const world::Player& player_;
    const world::Map& map_;
    render::FrameBuffer& frame_buffer_;
    const TextureStorage& textures_;
    const ViewSettings& view_settings_;
    const vector<double>& wallDepths;
    int frame_width_, frame_height_;

    const double eps = 1e-8;

public:

    SpriteRenderer(const world::Player& player, const world::Map& map,
                   render::FrameBuffer& frame_buffer, const TextureStorage& textures, const ViewSettings& view_settings,
                   const vector<double>& wallDepths) :
        player_(player),
        map_(map),
        frame_buffer_(frame_buffer),
        textures_(textures),
        view_settings_(view_settings),
        wallDepths(wallDepths),
        frame_width_(static_cast<int>(frame_buffer_.width())),
        frame_height_(static_cast<int>(frame_buffer_.height())) {}

    void renderSprites() const;

    };
}
