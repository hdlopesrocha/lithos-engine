#version 460 core

// Input from the vertex array
layout(location = 0) in vec3 position;   

uniform mat4 model;      
uniform mat4 view;       
uniform mat4 projection; 

void main() {
    gl_Position = vec4(position, 1.0);
}