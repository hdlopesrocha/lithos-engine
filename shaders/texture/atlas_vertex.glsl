#version 460 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 vTexCoord;
uniform mat4 model;
uniform vec2 tileSize;
uniform vec2 tileOffset;

void main() {
    vTexCoord = texCoord*tileSize+tileOffset;
    gl_Position = model * vec4( position, 0.0, 1.0);
}