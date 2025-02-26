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
	mat4 world;
	mat4 viewProjection;
	mat4 matrixShadow;
	vec4 lightDirection;
	vec4 cameraPosition;
    vec4 floatData;
    uvec4 uintData; 
};


#define PARALLAX_FLAG  0x01  
#define SHADOW_FLAG    0x02  
#define DEBUG_FLAG     0x04  
#define LIGHT_FLAG     0x08  
#define TRIPLANAR_FLAG 0x10  
#define DEPTH_FLAG     0x20  
#define OVERRIDE_FLAG  0x40  
#define TESSELATION_FLAG  0x80  

#define OPACITY_FLAG  0x01  


bool parallaxEnabled  = (uintData.x & uint(PARALLAX_FLAG)) != 0u;
bool shadowEnabled    = (uintData.x & uint(SHADOW_FLAG)) != 0u;
bool debugEnabled     = (uintData.x & uint(DEBUG_FLAG)) != 0u;
bool lightEnabled     = (uintData.x & uint(LIGHT_FLAG)) != 0u;
bool triplanarEnabled = (uintData.x & uint(TRIPLANAR_FLAG)) != 0u;
bool depthEnabled     = (uintData.x & uint(DEPTH_FLAG)) != 0u;
bool overrideEnabled  = (uintData.x & uint(OVERRIDE_FLAG)) != 0u;
bool tesselationEnabled  = (uintData.x & uint(TESSELATION_FLAG)) != 0u;

bool opacityEnabled  = (uintData.y & uint(OPACITY_FLAG)) != 0u;


uint overrideTexture  = uintData.w;
float time = floatData.x;
