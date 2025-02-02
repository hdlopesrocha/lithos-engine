struct TextureProperties {
    float parallaxScale;
    float parallaxMinLayers;
    float parallaxMaxLayers;
    float shininess;
    float specularStrength;
    float refractiveIndex;
    vec2 textureScale;
};

struct ShadowProperties {
    float lightAmount;
    float shadowAmount;
};