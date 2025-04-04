#version 460 core
in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor0; // color
layout(location = 1) out vec4 FragColor1; // normal
layout(location = 2) out vec4 FragColor2; // opacity

uniform sampler2DArray textures[3];
uniform uint atlasTexture;

uniform bool filterOpacity;

void main() {    
    vec4 opacity = texture(textures[2], vec3(vTexCoord,atlasTexture));
    if(filterOpacity && opacity.r == 0.0) {
        discard;
    }

    FragColor0 = texture(textures[0], vec3(vTexCoord,atlasTexture));
    FragColor1 = texture(textures[1], vec3(vTexCoord,atlasTexture));
    FragColor2 = opacity;

}