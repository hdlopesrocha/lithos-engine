#version 460 core

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint brushIndex;     
layout(location = 4) in mat4 instanceModel; 

#include<structs.glsl>
#include<functions.glsl>

out uint vTextureIndex;
out vec2 vTextureCoord;
out vec3 vPosition;
out vec3 vNormal;
out TextureProperties vProps;

uniform vec3 lightDirection;  
uniform mat4 modelViewProjection; 
uniform TextureProperties brushes[25];
uniform uint brushTextures[25];

uniform int layer;

void main() {
    if(layer > 0) {
        vTextureCoord = textureCoord;
        vTextureIndex = brushTextures[brushIndex];
        vProps = brushes[brushIndex];
        
        //vPosition = (instanceModel*vec4(position, 1.0)).xyz;
        vPosition = position;
        vNormal = normal;
    }
    gl_Position = vec4(vPosition, 1.0);
}