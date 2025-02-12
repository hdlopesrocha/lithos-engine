#version 460 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2DArray textureSampler;

in flat int Layer;



void main() {    

    vec4 color = texture(textureSampler, vec3(TexCoord,Layer));

    if(Layer == 2) {
        FragColor = vec4(color.r,color.r,color.r,1.0);
    }else {
        FragColor = color;
    }
    //FragColor = vec4(vec3(factor), 1.0);
}