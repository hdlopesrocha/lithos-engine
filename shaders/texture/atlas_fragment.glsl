#version 460 core
in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor0; // color
layout(location = 1) out vec4 FragColor1; // normal
layout(location = 2) out vec4 FragColor2; // bump
layout(location = 3) out vec4 FragColor3; // opacity

uniform sampler2DArray textureSampler;
uniform bool filterOpacity;

void main() {    
    vec4 opacity = texture(textureSampler, vec3(vTexCoord,3));
    if(filterOpacity && opacity.r == 0.0) {
        discard;
    }

    FragColor0 = texture(textureSampler, vec3(vTexCoord,0));
    FragColor1 = texture(textureSampler, vec3(vTexCoord,1));
    FragColor2 = texture(textureSampler, vec3(vTexCoord,2));
    FragColor3 = opacity;

}