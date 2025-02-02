#version 460 core
in vec2 TexCoord;
out vec4 FragColor;


uniform int perlinScale;
uniform float perlinTime;
uniform int perlinIterations;
uniform int perlinLacunarity;
uniform float brightness;
uniform float contrast;
uniform vec4 color;


in flat int Layer;

#include<perlin.glsl>
#include<structs.glsl>
#include<functions_fragment.glsl>

float applyBrightnessContrast(float factor) {
    return clamp((factor - 0.5) * contrast + 0.5 + brightness, 0.0, 1.0);
}


void main() {    
    if(Layer == 0) {
        FragColor = color;
    }
    else {
        vec3 n = fbmd(TexCoord, vec2(perlinScale), perlinIterations, 0, perlinTime, 0.5, perlinLacunarity, 0.5, 0.0, 0.0);
       if(Layer == 1) {
            vec3 normal = normalize(vec3(n.z,n.y, 1.0));
            normal = visual(normal);
            FragColor = vec4(normal, 1.0);
       } else {
            float factor = n.x;
            factor = applyBrightnessContrast(factor);
            FragColor = vec4(vec3(factor), 1.0);
       }
    }
}