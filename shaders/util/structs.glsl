struct TextureProperties {
    float parallaxScale;
    float parallaxMinLayers;
    float parallaxMaxLayers;
    float parallaxFade;
    float parallaxRefine;
    float shininess;
    float specularStrength;
    float refractiveIndex;
    vec2 textureScale;
};

struct ShadowProperties {
    float lightAmount;
    float shadowAmount;
};