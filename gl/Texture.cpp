#include "gl.hpp"

Texture::Texture() {
    this->texture = 0;
    this->parallaxScale = 0;
    this->parallaxMinLayers = 0;
    this->parallaxMaxLayers = 0;
    this->shininess = 0;
    this->specularStrength = 0;
}

Texture::Texture(TextureArray texture) : Texture() {
    this->texture = texture;
}

Texture::Texture(TextureArray texture, float parallaxScale, float parallaxMinLayers, float parallaxMaxLayers, float shininess, float specularStrength) {
    this->texture = texture;
    this->parallaxScale = parallaxScale;
    this->parallaxMinLayers = parallaxMinLayers;
    this->parallaxMaxLayers = parallaxMaxLayers;
    this->shininess = shininess;
    this->specularStrength = specularStrength;
}

