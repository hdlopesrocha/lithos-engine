#version 460 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2DArray baseTexture;
uniform sampler2DArray overlayTexture;

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

    vec4 baseColor = texture(baseTexture, vec3(TexCoord,Layer));
    vec4 overlayColor = texture(overlayTexture, vec3(TexCoord,Layer));

    vec4 color = baseColor*(factor)  + overlayColor*(1.0-factor);

    FragColor = color;
    //FragColor = vec4(vec3(factor), 1.0);
}