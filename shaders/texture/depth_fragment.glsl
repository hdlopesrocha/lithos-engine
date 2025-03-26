#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textureSampler;
#include<depth.glsl>


void main() {
    float d = texture(textureSampler, TexCoord).r;

    float near = 0.1;
    float far = 1024.0;
    float d2 = linearizeDepth(d, near, far)/far;

    FragColor = vec4(d2, d2 ,d2 ,1.0);
}