#version 460 core

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint textureIndex;    
layout(location = 4) in vec3 tangent;    
layout(location = 5) in vec3 bitangent;    


out float vTextureWeights[10];
out vec2 vTextureCoord;
out vec3 vPosition;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;


uniform vec3 lightDirection;  
uniform uint triplanarEnabled;

#include<functions.glsl>

void main() {

    vTextureCoord = textureCoord;
   
    for (int i = 0; i < 10; ++i) {
        vTextureWeights[i] = 0.0;
    }

    vPosition = position;
    vTextureWeights[textureIndex] = 1.0;
    vNormal = normal;
    vTangent = tangent;
    vBitangent = bitangent;

    gl_Position = vec4(position, 1.0);
}