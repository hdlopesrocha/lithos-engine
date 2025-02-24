#define PI 3.1415926535897932384626433832795

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


layout(std140, binding = 0) uniform UniformBlock {
	mat4 model;
	mat4 modelViewProjection;
	mat4 matrixShadow;
	vec4 lightDirection;
	vec4 cameraPosition;
    vec4 timeAndPadding;
	uint parallaxEnabled;
	uint shadowEnabled;
	uint debugEnabled;
	uint lightEnabled;
	uint triplanarEnabled;
	uint depthEnabled;
    uint overrideEnabled;
    uint overrideTexture;
    uint padding;
};
