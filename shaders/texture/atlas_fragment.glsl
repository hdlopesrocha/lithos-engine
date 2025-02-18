#version 460 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2DArray textureSampler;

in flat int Layer;

void main() {    
    vec4 opacity = texture(textureSampler, vec3(TexCoord,2));
    if(opacity.r < 0.5) {
        discard;
    }

    if(Layer == 2) {
        FragColor = vec4(1.0,1.0,1.0,1.0);
    }else {
        FragColor = texture(textureSampler, vec3(TexCoord,Layer));
    }
    //FragColor = vec4(vec3(factor), 1.0);
}