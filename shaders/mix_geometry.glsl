#version 460 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

out vec2 TexCoord;
out flat int Layer;
uniform int layerIndex; // Use this to select a layer dynamically


void main() {
    for (int i = 0; i < 3; i++) {
        gl_Position = gl_in[i].gl_Position;
        TexCoord = gl_in[i].gl_Position.xy;
        gl_Layer = layerIndex; 
        Layer = layerIndex;
        EmitVertex();
    }
    EndPrimitive();
}