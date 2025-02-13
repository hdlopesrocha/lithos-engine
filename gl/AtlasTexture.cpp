#include "gl.hpp"

AtlasTexture::AtlasTexture(Texture * texture, std::vector<Tile> tiles) {
    this->texture = texture;
    this->tiles = tiles;
}

