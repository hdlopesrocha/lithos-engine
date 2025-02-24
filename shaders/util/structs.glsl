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
    uvec4 data; 
};


#define PARALLAX_FLAG  0x01  
#define SHADOW_FLAG    0x02  
#define DEBUG_FLAG     0x04  
#define LIGHT_FLAG     0x08  
#define TRIPLANAR_FLAG 0x10  
#define DEPTH_FLAG     0x20  
#define OVERRIDE_FLAG  0x40  

bool parallaxEnabled  = (data.x & uint(PARALLAX_FLAG)) != 0u;
bool shadowEnabled    = (data.x & uint(SHADOW_FLAG)) != 0u;
bool debugEnabled     = (data.x & uint(DEBUG_FLAG)) != 0u;
bool lightEnabled     = (data.x & uint(LIGHT_FLAG)) != 0u;
bool triplanarEnabled = (data.x & uint(TRIPLANAR_FLAG)) != 0u;
bool depthEnabled     = (data.x & uint(DEPTH_FLAG)) != 0u;
bool overrideEnabled  = (data.x & uint(OVERRIDE_FLAG)) != 0u;
uint overrideTexture  = data.w;
