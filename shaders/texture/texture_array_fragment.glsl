#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2DArray textures[3];

uniform uint targetLayer;
uniform uint index;
uniform bool mono;

void main() {
    if(mono) {
        float color = texture(textures[targetLayer], vec3(TexCoord, index)).r;
        FragColor = vec4(color, color, color, 1.0);
    }
    else {
        FragColor = texture(textures[targetLayer], vec3(TexCoord, index));
    }
}