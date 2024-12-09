#version 460 core

in vec3 fragColor; // Interpolated color from the vertex shader
out vec4 color;    // Final fragment color

void main() {
    color = vec4(fragColor, 1.0); // Output color with alpha = 1.0
}