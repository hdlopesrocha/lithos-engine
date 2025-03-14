#version 460 core
in vec2 vTexCoord;

layout(location = 0) out vec4 FragColor0; // First render target (color)
layout(location = 1) out vec4 FragColor1; // Second render target (normals)
layout(location = 2) out vec4 FragColor2; // Third render target (bump)

uniform sampler2DArray textures[3];
uniform uint baseTexture;
uniform uint overlayTexture;

uniform int perlinScale;
uniform float perlinTime;
uniform int perlinIterations;
uniform int perlinLacunarity;
uniform float brightness;
uniform float contrast;

#include<perlin.glsl>

float applyBrightnessContrast(float factor) {
    return clamp((factor - 0.5) * contrast + 0.5 + brightness, 0.0, 1.0);
}


void main() {    
    float factor = fbm(vTexCoord, vec2(perlinScale), perlinIterations, 0, perlinTime, 0.5, perlinLacunarity, 0.0, 0.0);
    factor = applyBrightnessContrast(factor);
    for(int layer = 0; layer < 3 ; ++layer) {
        vec4 baseColor = texture(textures[layer], vec3(vTexCoord,baseTexture));
        vec4 overlayColor = texture(textures[layer], vec3(vTexCoord,overlayTexture));

        vec4 color = baseColor*(factor)  + overlayColor*(1.0-factor);

        if(layer == 0) {
            FragColor0 = color;
        }else if(layer == 1) {
            FragColor1 = color;
        }else {
            FragColor2 = vec4(color.r,color.r,color.r,1.0);
        }
    }
}