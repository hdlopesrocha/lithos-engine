#version 460 core

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint textureIndex;     
layout(location = 4) in float parallaxScale;     
layout(location = 5) in float parallaxMinLayers;     
layout(location = 6) in float parallaxMaxLayers;     
layout(location = 7) in float shininess;     
layout(location = 8) in float specularStrength;     

#include<functions.glsl>

out float vTextureWeights[20];
out vec2 vTextureCoord;
out vec3 vPosition;
out vec3 vNormal;
out TextureProperties vProps;

uniform vec3 lightDirection;  
uniform uint triplanarEnabled;

void main() {

    vTextureCoord = textureCoord;
   
    for (int i = 0; i < 20; ++i) {
        vTextureWeights[i] = 0.0;
    }

    vPosition = position;
    vTextureWeights[textureIndex] = 1.0;
    vNormal = normal;
    vProps.parallaxScale = parallaxScale;
    vProps.parallaxMinLayers = parallaxMinLayers;
    vProps.parallaxMaxLayers = parallaxMaxLayers;
    vProps.shininess = shininess;
    vProps.specularStrength = specularStrength;
    gl_Position = vec4(position, 1.0);
}