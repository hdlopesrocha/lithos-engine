#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textureSampler;
uniform float near;
uniform float far;

void main() {
    vec4 color = texture(textureSampler, TexCoord);
    FragColor = color;
}