#version 460 core
#include<structs.glsl>

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint textureIndex;     
layout(location = 4) in mat4 instanceModel; 

#include<functions.glsl>

out vec2 vTextureCoord;
out vec3 vPosition;
out vec3 vNormal;
out TextureProperties vProps;
out vec4 vLightViewPosition;

uniform TextureProperties brushes[25];

void main() {
    vTextureCoord = textureCoord;
    vProps = brushes[textureIndex];
    vPosition = (instanceModel*vec4(position, 1.0)).xyz;
    vNormal = normal;

    gl_Position = modelViewProjection * vec4(vPosition, 1.0);    
    vLightViewPosition = matrixShadow * vec4(vPosition, 1.0);  

}