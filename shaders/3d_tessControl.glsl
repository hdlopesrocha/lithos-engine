#version 460 core
#include<structs.glsl>

layout(vertices = 3) out;

in uint vTextureIndex[];
in vec2 vTextureCoord[];
in vec3 vPosition[];
in vec3 vNormal[];
in TextureProperties vProps[];
in mat4 vModel[];
#include<functions.glsl>

flat out uvec3 tcTextureIndices[];
out vec2 tcTextureCoord[];
out vec3 tcNormal[];
out vec3 tcPosition[];
out TextureProperties tcProps[];
out mat4 tcModel[];


void main() {
     vec3 patchCentroid = (vPosition[0] + vPosition[1] + vPosition[2]) / 3.0;
    float distance = length(cameraPosition.xyz - patchCentroid);
    float tessellationFactor = clamp(100.0 / distance, 0.0, tesselationEnabled? 8.0 : 1.0); // Adjust these numbers to fit your scene
    gl_TessLevelOuter[0] = tessellationFactor;
    gl_TessLevelOuter[1] = tessellationFactor;
    gl_TessLevelOuter[2] = tessellationFactor;
    gl_TessLevelInner[0] = tessellationFactor;

    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID]; 
    tcModel[gl_InvocationID] = vModel[gl_InvocationID]; 
    
    if(!depthEnabled) {
        tcNormal[gl_InvocationID] = vNormal[gl_InvocationID];
        
        tcProps[gl_InvocationID] = vProps[gl_InvocationID];

        tcTextureCoord[gl_InvocationID] = vTextureCoord[gl_InvocationID];
        for (int i = 0; i < 3; ++i) {
            tcTextureIndices[gl_InvocationID][i] = vTextureIndex[i];
        }
    }
}