#version 460 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 vTexCoord;
uniform mat3 model;

void main() {
    vTexCoord = texCoord;
    gl_Position = vec4(model * vec3(position, 0.0), 1.0);
}