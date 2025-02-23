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
	float time;
	int parallaxEnabled;
	int shadowEnabled;
	int debugEnabled;
	int lightEnabled;
	int triplanarEnabled;
	int layer;
	int padding; 
};
