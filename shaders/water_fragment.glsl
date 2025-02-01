#version 460 core
in vec2 TexCoord;
out vec4 FragColor;


uniform int perlinScale;
uniform float perlinTime;
uniform int perlinIterations;
uniform int perlinLacunarity;
uniform float brightness;
uniform float contrast;
in flat int Layer;

#include<perlin.glsl>

float applyBrightnessContrast(float factor) {
    return clamp((factor - 0.5) * contrast + 0.5 + brightness, 0.0, 1.0);
}



void main() {    
    float factor = fbm(TexCoord, vec2(perlinScale), perlinIterations, 0, perlinTime, 0.5, perlinLacunarity, 0.0, 0.0);
    factor = applyBrightnessContrast(factor);

    FragColor = vec4(vec3(factor), 1.0);
}