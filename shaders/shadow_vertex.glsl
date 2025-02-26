#version 460 core

// Input from the vertex array
layout(location = 0) in vec3 position;   

uniform mat4 modelViewProjection; 

void main() {
    gl_Position = modelViewProjection * vec4(position, 1.0);    
}