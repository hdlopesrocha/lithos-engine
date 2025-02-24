#version 460 core
#include<structs.glsl>

// Input from the vertex array
layout(location = 0) in vec3 position;   

void main() {
    gl_Position = modelViewProjection * vec4(position, 1.0);    
}