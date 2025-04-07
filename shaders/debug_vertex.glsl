#version 460 core

#include<structs.glsl>
#include<uniforms.glsl>

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint brushIndex;     
layout(location = 4) in float shift; 
layout(location = 5) in uint animation; 
layout(location = 6) in mat4 model; 


void main() {
    mat4 mvp = viewProjection * world * model;
    gl_Position = mvp * vec4(position, 1.0);    
}