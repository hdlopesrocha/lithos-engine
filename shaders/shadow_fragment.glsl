#version 460 core

out vec4 color;    // Final fragment color
in float depth;
void main() {
    color = vec4(depth,depth,depth,1.0);
}


