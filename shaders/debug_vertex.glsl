#version 460 core

#include<structs.glsl>
#include<uniforms.glsl>

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint brushIndex;     
layout(location = 4) in mat4 model; 

layout(location = 8) in float sdf0;
layout(location = 9) in float sdf1;
layout(location = 10) in float sdf2;
layout(location = 11) in float sdf3;
layout(location = 12) in float sdf4;
layout(location = 13) in float sdf5;
layout(location = 14) in float sdf6;
layout(location = 15) in float sdf7;
out float distance;
out float scale;

void main() {
    float sdf[8];
    sdf[0] = sdf0;
    sdf[1] = sdf1;
    sdf[2] = sdf2;
    sdf[3] = sdf3;
    sdf[4] = sdf4;
    sdf[5] = sdf5;
    sdf[6] = sdf6;
    sdf[7] = sdf7;

    uint ix = clamp(uint(position.x),0u,1u);
    uint iy = clamp(uint(position.y),0u,1u);
    uint iz = clamp(uint(position.z),0u,1u);
    uint i = (ix << 2) + (iy << 1) + iz;
    distance = sdf[i];
    scale = length(model[0].xyz);
    mat4 mvp = viewProjection * world * model;
    gl_Position = mvp * vec4(position, 1.0);    
}