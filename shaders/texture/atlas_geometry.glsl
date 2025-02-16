#version 460 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3 * 3) out;

out vec2 TexCoord;
out flat int Layer;

in vec2 vTexCoord[]; // Receive from Vertex Shader

void main() {
    for (int l = 0; l < 3; l++) {
        for (int i = 0; i < 3; i++) {
            gl_Position = gl_in[i].gl_Position;
            TexCoord = vTexCoord[i];
            gl_Layer = l; 
            Layer = l;
            EmitVertex();
        }
        EndPrimitive();
    }
}