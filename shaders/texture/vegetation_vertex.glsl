#version 460 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 vTexCoord;
uniform mat4 model;

void main() {
    vTexCoord = texCoord;
    gl_Position = model * vec4(position, 0.0, 1.0);
}