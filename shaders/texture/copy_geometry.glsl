#version 460 core

layout(triangles) in;  
layout(triangle_strip, max_vertices = 3) out;  

in vec2 texCoord[];  
out vec2 gs_TexCoord;

uniform int targetLayer;  

void main() {
    gl_Layer = targetLayer;

    for (int i = 0; i < 3; i++) {
        gs_TexCoord = texCoord[i];
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}