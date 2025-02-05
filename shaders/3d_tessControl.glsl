#version 460 core

layout(vertices = 3) out;

#include<structs.glsl>
#include<functions.glsl>

in float vTextureWeights[][20];
in vec2 vTextureCoord[];
in vec3 vPosition[];
in vec3 vNormal[];
in TextureProperties vProps[];

out float tcTextureWeights[][20];
out vec2 tcTextureCoord[];
out vec3 tcNormal[];
out vec3 tcPosition[];
out TextureProperties tcProps[];

uniform vec3 cameraPosition; 

void main() {
     vec3 patchCentroid = (vPosition[0] + vPosition[1] + vPosition[2]) / 3.0;
    float distance = length(cameraPosition - patchCentroid);
    float tessellationFactor = clamp(100.0 / distance, 0.0, 1.0); // Adjust these numbers to fit your scene
    gl_TessLevelOuter[0] = tessellationFactor;
    gl_TessLevelOuter[1] = tessellationFactor;
    gl_TessLevelOuter[2] = tessellationFactor;
    gl_TessLevelInner[0] = tessellationFactor;

    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID]; 
    tcNormal[gl_InvocationID] = vNormal[gl_InvocationID];
    
    tcProps[gl_InvocationID] = vProps[gl_InvocationID];

    tcTextureCoord[gl_InvocationID] = vTextureCoord[gl_InvocationID];
    for (int i = 0; i < 20; ++i) {
        tcTextureWeights[gl_InvocationID][i] = vTextureWeights[gl_InvocationID][i];
    }

}