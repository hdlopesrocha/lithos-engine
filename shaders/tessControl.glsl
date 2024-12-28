#version 460 core

layout(vertices = 3) out;

in float vTextureWeights[][16];
in vec2 vTextureCoord[];
in vec3 vNormal[];
in vec3 vPosition[];

out float tcTextureWeights[][16];
out vec2 tcTextureCoord[];
out vec3 tcNormal[];
out vec3 tcPosition[];;


void main() {
    // Set tessellation levels (adjust these as needed)
    gl_TessLevelInner[0] = 8.0; // or higher for more detail
    gl_TessLevelOuter[0] = 8.0;
    gl_TessLevelOuter[1] = 8.0;
    gl_TessLevelOuter[2] = 8.0; // For triangle patches

    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID]; 
    tcNormal[gl_InvocationID] = vNormal[gl_InvocationID];
    tcTextureCoord[gl_InvocationID] = vTextureCoord[gl_InvocationID];
    for (int i = 0; i < 16; ++i) {
        tcTextureWeights[gl_InvocationID][i] = vTextureWeights[gl_InvocationID][i];
    }

}