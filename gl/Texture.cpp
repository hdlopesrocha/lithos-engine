#include "gl.hpp"

Texture::Texture(Image texture) {
    this->texture = texture;
    this->normal = 0;
    this->bump = 0;
    this->parallaxScale = 0;
    this->parallaxMinLayers = 0;
    this->parallaxMaxLayers = 0;
    this->shininess = 0;
}

Texture::Texture(Image texture, Image normal, Image bump, float parallaxScale, float parallaxMinLayers, float parallaxMaxLayers, float shininess) {
    this->texture = texture;
    this->normal = normal;
    this->bump = bump;
    this->parallaxScale = parallaxScale;
    this->parallaxMinLayers = parallaxMinLayers;
    this->parallaxMaxLayers = parallaxMaxLayers;
    this->shininess = shininess;
}

