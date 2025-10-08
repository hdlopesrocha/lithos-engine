#version 460 core

#include<structs.glsl>
#include<uniforms.glsl>
#include<functions.glsl>

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in int brushIndex;     
layout(location = 4) in mat4 model; 

layout(location = 8) in vec4 sdf1;     // 16 bytes
layout(location = 9) in vec4 sdf2;     // 16 bytes
layout(location = 10) in int brushIndex2;     

out float distance;
out float scale;
out vec4 vertexColor;     
out float brush;
out vec2 texCoord;

void main() {
    float sdf[8];
    sdf[0] = sdf1[0];
    sdf[1] = sdf1[1];
    sdf[2] = sdf1[2];
    sdf[3] = sdf1[3];
    sdf[4] = sdf2[0];
    sdf[5] = sdf2[1];
    sdf[6] = sdf2[2];
    sdf[7] = sdf2[3];

    uint ix = clamp(uint(position.x),0u,1u);
    uint iy = clamp(uint(position.y),0u,1u);
    uint iz = clamp(uint(position.z),0u,1u);
    uint i = (ix << 2) + (iy << 1) + iz;
    distance = sdf[i];
    scale = length(model[0].xyz);
    mat4 mvp = viewProjection * world * model;
    gl_Position = mvp * vec4(position, 1.0);    
    texCoord = textureCoord;
    vertexColor = vec4(brushColor(brushIndex2),1.0);
    brush = float(brushIndex2);
}