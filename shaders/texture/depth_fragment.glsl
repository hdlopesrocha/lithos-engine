#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textureSampler;
#include<depth.glsl>


void main() {
    float d = texture(textureSampler, TexCoord).r;
    FragColor = vec4(d, d ,d ,1.0);
}