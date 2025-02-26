#version 460 core
#include<structs.glsl>

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint brushIndex;     
layout(location = 4) in mat4 model; 

#include<functions.glsl>


out uint vTextureIndex;
out vec2 vTextureCoord;
out vec3 vPosition;
out vec3 vNormal;
out TextureProperties vProps;
out mat4 vModel;

uniform TextureProperties brushes[25];
uniform uint brushTextures[25];


void main() {
    vTextureIndex = brushTextures[brushIndex];
    vTextureCoord = textureCoord;
    vModel = world*model;
    vec3 iPosition = position;
    float freq = 1.0/ PI;

    vec3 wPosition = (vModel*vec4(iPosition, 1.0)).xyz;

    if(opacityEnabled && iPosition.y > 0.0) {
        iPosition.x += sin(wPosition.x*freq + time);
        iPosition.z += cos(wPosition.z*freq + time);
    } 

    if(!depthEnabled) {
        vProps = brushes[brushIndex];
        vNormal = normal;
    }
    vPosition = (vModel*vec4(iPosition, 1.0)).xyz;
    gl_Position = vec4(vPosition, 1.0);
}