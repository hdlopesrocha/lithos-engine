#version 460 core

// Input from the vertex array
layout(location = 0) in vec3 position; // Vertex position (x, y, z)
layout(location = 1) in vec3 color;    // Vertex color (r, g, b)

// Output to the fragment shader
out vec3 fragColor;

uniform mat4 model;      // Model transformation matrix
uniform mat4 view;       // View transformation matrix
uniform mat4 projection; // Projection transformation matrix

void main() {
    // Pass color to the fragment shader
    fragColor = color;

    // Calculate the final position of the vertex
    gl_Position = projection * view * model * vec4(position, 1.0);
}