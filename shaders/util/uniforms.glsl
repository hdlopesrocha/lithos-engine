#define SHADOW_MATRIX_COUNT 3


layout(std140, binding = 0) uniform UniformBlock {
	mat4 world;
	mat4 viewProjection;
	mat4 matrixShadow[SHADOW_MATRIX_COUNT];
	vec4 lightDirection;
	vec4 cameraPosition;
    vec4 floatData[2];
	uvec4 uintData; 
	vec4 ambientColor;
};


#define PARALLAX_FLAG  		0x01  
#define SHADOW_FLAG    		0x02  
#define DEBUG_FLAG     		0x04  
#define LIGHT_FLAG     		0x08  
#define DEPTH_FLAG     		0x10  
#define OVERRIDE_FLAG 		0x20  
#define TESSELATION_FLAG 	0x40  
#define OPACITY_FLAG  		0x80  

#define BILLBOARD_FLAG  0x01  


bool parallaxEnabled  = (uintData.x & uint(PARALLAX_FLAG)) != 0u;
bool shadowEnabled    = (uintData.x & uint(SHADOW_FLAG)) != 0u;
bool debugEnabled     = (uintData.x & uint(DEBUG_FLAG)) != 0u;
bool lightEnabled     = (uintData.x & uint(LIGHT_FLAG)) != 0u;

bool depthEnabled     = (uintData.x & uint(DEPTH_FLAG)) != 0u;
bool overrideEnabled  = (uintData.x & uint(OVERRIDE_FLAG)) != 0u;
bool tesselationEnabled  = (uintData.x & uint(TESSELATION_FLAG)) != 0u;
bool opacityEnabled  = (uintData.x & uint(OPACITY_FLAG)) != 0u;

bool billboardEnabled  = (uintData.y & uint(BILLBOARD_FLAG)) != 0u;

uint debugMode  = uintData.z;
uint overrideBrush  = uintData.w;
float time = floatData[0].x;
float blendSharpness = floatData[0].y;
float parallaxDistance = floatData[0].z;
float parallaxPower = floatData[0].w;
float near = floatData[1].x;
float far = floatData[1].y;
vec3 uAmbientColor = ambientColor.xyz;
float uAmbientIntensity = ambientColor.w;