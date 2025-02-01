#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2DArray textureSampler;
uniform int textureLayer;

void main() {
    vec4 color = texture(textureSampler, vec3(TexCoord, textureLayer));
    FragColor = color;
}