#include "wolf/texture/TextureStore.hpp"

#include <fstream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace wolf
{
namespace
{

std::string nextToken(std::istream& input)
{
    std::string token;
    while (input >> token)
    {
        if (!token.empty() && token.front() == '#')
        {
            input.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        return token;
    }
    throw std::runtime_error("Unexpected end of PPM file");
}

int nextNumber(std::istream& input, const std::string& path)
{
    const std::string token = nextToken(input);
    std::size_t consumed = 0;
    int value = 0;
    try
    {
        value = std::stoi(token, &consumed);
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("Invalid numeric value in PPM file: " + path);
    }
    if (consumed != token.size())
    {
        throw std::runtime_error("Invalid numeric value in PPM file: " + path);
    }
    return value;
}

} // namespace

void TextureStore::insert(int textureId, Texture texture)
{
    if (textureId <= 0)
    {
        throw std::invalid_argument("Texture identifier must be positive");
    }
    if (!texture.valid())
    {
        throw std::invalid_argument("Cannot store an invalid texture");
    }
    textures_.insert_or_assign(textureId, std::move(texture));
}

void TextureStore::loadPpm(int textureId, const std::string& path,
                           std::optional<Texture::Pixel> transparentColor)
{
    std::ifstream input(path);
    if (!input)
    {
        throw std::runtime_error("Cannot open texture: " + path);
    }
    if (nextToken(input) != "P3")
    {
        throw std::runtime_error("Only text P3 PPM textures are supported: " + path);
    }

    Texture texture;
    texture.width = nextNumber(input, path);
    texture.height = nextNumber(input, path);
    const int maximum = nextNumber(input, path);
    if (texture.width <= 0 || texture.height <= 0 || maximum <= 0 || maximum > 65535)
    {
        throw std::runtime_error("Invalid PPM header: " + path);
    }

    const std::size_t pixelCount = static_cast<std::size_t>(texture.width)
        * static_cast<std::size_t>(texture.height);
    texture.pixels.reserve(pixelCount);
    for (std::size_t index = 0; index < pixelCount; ++index)
    {
        const int red = nextNumber(input, path);
        const int green = nextNumber(input, path);
        const int blue = nextNumber(input, path);
        if (red < 0 || red > maximum || green < 0 || green > maximum
            || blue < 0 || blue > maximum)
        {
            throw std::runtime_error("PPM color component is out of range: " + path);
        }

        const auto scale = [maximum](int component)
        {
            return static_cast<Texture::Pixel>((component * 255 + maximum / 2) / maximum);
        };
        Texture::Pixel pixel = 0xFF000000U | (scale(red) << 16U) | (scale(green) << 8U) | scale(blue);
        if (transparentColor && pixel == *transparentColor)
        {
            pixel = 0;
        }
        texture.pixels.push_back(pixel);
    }
    insert(textureId, std::move(texture));
}

bool TextureStore::contains(int textureId) const
{
    return textures_.contains(textureId);
}

const Texture& TextureStore::get(int textureId) const
{
    const auto iterator = textures_.find(textureId);
    if (iterator == textures_.end())
    {
        throw std::out_of_range("Unknown texture identifier");
    }
    return iterator->second;
}

} // namespace wolf
