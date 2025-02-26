#version 460 core
in vec2 vTexCoord;

layout(location = 0) out vec4 FragColor0; // First render target (color)
layout(location = 1) out vec4 FragColor1; // Second render target (normals)
layout(location = 2) out vec4 FragColor2; // Third render target (bump)

uniform int perlinScale;
uniform float perlinTime;
uniform int perlinIterations;
uniform int perlinLacunarity;
uniform float brightness;
uniform float contrast;
uniform vec4 color;

#include<perlin.glsl>
#include<functions.glsl>

float applyBrightnessContrast(float factor) {
    return clamp((factor - 0.5) * contrast + 0.5 + brightness, 0.0, 1.0);
}


void main() {    
    FragColor0 = color;
        
    vec3 n = fbmd(vTexCoord, vec2(perlinScale), perlinIterations, 0, perlinTime, 0.5, perlinLacunarity, 0.5, 0.0, 0.0);

    vec3 normal = normalize(vec3(n.z,n.y, 1.0));
    normal = visual(normal);
    FragColor1 = vec4(normal, 1.0);

    float factor = n.x;
    factor = applyBrightnessContrast(factor);
    FragColor2 = vec4(vec3(factor), 1.0);   
}