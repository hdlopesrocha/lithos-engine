#version 460 core

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint textureIndex;     

#include<structs.glsl>
#include<functions.glsl>

out vec2 vTextureCoord;
out vec3 vPosition;
out vec3 vNormal;
out TextureProperties vProps;
out vec4 lightViewPosition;

uniform mat4 modelViewProjection; 
uniform TextureProperties brushes[20];
uniform mat4 matrixShadow; 

void main() {
    vTextureCoord = textureCoord;
    vProps = brushes[textureIndex];
    vPosition = position;
    vNormal = normal;

    gl_Position = modelViewProjection * vec4(position, 1.0);    
    lightViewPosition = matrixShadow * vec4(position, 1.0);  

}