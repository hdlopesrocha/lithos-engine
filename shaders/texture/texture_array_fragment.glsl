#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2DArray textureSampler;
uniform int textureLayer;

void main() {
    vec4 color = texture(textureSampler, vec3(TexCoord, textureLayer));
    if(textureLayer == 2) {
        FragColor = vec4(color.r, color.r, color.r, 1.0);
    }
    else {
        FragColor = color;
    }
}