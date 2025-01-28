#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2DArray textureSampler;
uniform uint textureLayer;

void main() {
    vec4 color = texture(textureSampler, vec3(TexCoord,textureLayer));
    FragColor = vec4(color.rgb, 1.0);
}