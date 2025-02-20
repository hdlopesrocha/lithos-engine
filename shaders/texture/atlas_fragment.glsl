#version 460 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2DArray textureSampler;
uniform bool filterOpacity;

in flat int Layer;

void main() {    
    vec4 opacity = texture(textureSampler, vec3(TexCoord,3));
    if(filterOpacity && opacity.r < 0.2) {
        discard;
    }

    if(Layer == 3) {
        FragColor = opacity;
    }else {
        FragColor = texture(textureSampler, vec3(TexCoord,Layer));
    }
}