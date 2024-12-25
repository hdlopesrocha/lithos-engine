#version 460 core

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint textureIndex;    


out float vTextureWeights[16];
out vec2 vTextureCoord;
out vec3 vNormal;
out vec3 vPosition;


uniform vec3 lightDirection;     // Direction of the light (assumed to be normalized)


void main() {

    for (int i = 0; i < 16; ++i) {
        vTextureWeights[i] = 0.0;
    }

    vPosition = position;
    vTextureWeights[textureIndex] = 1.0;
    vTextureCoord = textureCoord;
    vNormal = normal;

    gl_Position = vec4(position, 1.0);
}