#define PI 3.1415926535897932384626433832795

struct TextureBrush {
    float parallaxScale;
    float parallaxMinLayers;
    float parallaxMaxLayers;
    float parallaxFade;
    float parallaxRefine;
    float shininess;
    float specularStrength;
    float refractiveIndex;
    vec2 textureScale;
    float alpha;
};

struct ShadowProperties {
    float shadowAmount;
};

struct Vertex {
    vec4 position;
    vec4 normal;
    vec2 texCoord;
    int brushIndex;
    int _pad0;
};

struct InstanceData {
    mat4 matrix;
    float shift;
	uint animation;
};
