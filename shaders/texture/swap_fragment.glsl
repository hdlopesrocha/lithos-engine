#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textureSampler;

void main() {
    vec4 color = texture(textureSampler, TexCoord);
    FragColor = color;
}