#version 430 core

in vec2 gs_TexCoord; 
out vec4 FragColor;

uniform sampler2DArray textureArray; 


uniform int sourceLayer; 

void main() {
    FragColor = texture(textureArray, vec3(gs_TexCoord, sourceLayer));  
}