#version 460 core
#include<structs.glsl>

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint brushIndex;     
layout(location = 4) in mat4 model; 

uniform TextureProperties brushes[25];
uniform uint brushTextures[25];
uniform mat4 world;
uniform mat4 viewProjection;

#include<functions.glsl>

out uint vTextureIndex;
out vec2 vTextureCoord;
out vec3 vPosition;
out vec3 vNormal;
out TextureProperties vProps;
out mat4 vModel;


void main() {
    vTextureIndex = brushTextures[brushIndex];
    vTextureCoord = textureCoord;
    vModel = world*model;
    vProps = brushes[brushIndex];
    vNormal = normal;
    vPosition = (vModel*vec4(position, 1.0)).xyz;
    gl_Position = viewProjection * vec4(vPosition, 1.0);    
}