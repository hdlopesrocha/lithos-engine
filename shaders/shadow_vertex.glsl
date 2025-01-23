#version 460 core

// Input from the vertex array
layout(location = 0) in vec3 position;   

uniform mat4 modelViewProjection; 
out float depth;

void main() {
    float far = 512.0;
    gl_Position = modelViewProjection * vec4(position, 1.0);    
    depth = gl_Position.z/ far;
}