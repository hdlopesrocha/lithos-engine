#version 460 core

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint textureIndex;     
 
#include<functions.glsl>

out float vTextureWeights[20];
out vec2 vTextureCoord;
out vec3 vPosition;
out vec3 vNormal;
out TextureProperties vProps;

uniform vec3 lightDirection;  
uniform mat4 modelViewProjection; 
uniform TextureProperties brushes[20];

void main() {

    vTextureCoord = textureCoord;
   
    for (int i = 0; i < 20; ++i) {
        vTextureWeights[i] = 0.0;
    }
    vTextureWeights[textureIndex] = 1.0;
    vProps = brushes[textureIndex];
    
    vPosition = position;
    vNormal = normal;
    gl_Position = vec4(position, 1.0);
}