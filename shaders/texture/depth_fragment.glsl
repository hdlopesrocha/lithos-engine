#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textureSampler;
uniform float near;
uniform float far;

#include<depth.glsl>


void main() {
    float d = texture(textureSampler, TexCoord).r;
    float d2 = linearizeDepth(d, near, far)/far;

    FragColor = vec4(d2, d2 ,d2 ,1.0);
}