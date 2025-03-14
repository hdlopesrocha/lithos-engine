#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2DArray textures[3];

uniform uint layer;
uniform uint index;

void main() {
    FragColor = texture(textures[layer], vec3(TexCoord, index));
}