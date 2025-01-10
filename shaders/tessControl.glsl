#version 460 core

layout(vertices = 3) out;

in float vTextureWeights[][10];
in vec2 vTextureCoord[];
in vec3 vPosition[];
in vec3 vNormal[];
in vec3 vTangent[];
in vec3 vBitangent[];

out float tcTextureWeights[][10];
out vec2 tcTextureCoord[];
out vec3 tcNormal[];
out vec3 tcPosition[];
out vec3 tcTangent[];
out vec3 tcBitangent[];

uniform vec3 cameraPosition; 

void main() {
     vec3 patchCentroid = (vPosition[0] + vPosition[1] + vPosition[2]) / 3.0;
    float distance = length(cameraPosition - patchCentroid);
    float tessellationFactor = clamp(100.0 / distance, 1.0, 16.0); // Adjust these numbers to fit your scene
    gl_TessLevelOuter[0] = tessellationFactor;
    gl_TessLevelOuter[1] = tessellationFactor;
    gl_TessLevelOuter[2] = tessellationFactor;
    gl_TessLevelInner[0] = tessellationFactor;

    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID]; 

    tcNormal[gl_InvocationID] = vNormal[gl_InvocationID];
    tcTangent[gl_InvocationID] = vTangent[gl_InvocationID];
    tcBitangent[gl_InvocationID] = vBitangent[gl_InvocationID];

    tcTextureCoord[gl_InvocationID] = vTextureCoord[gl_InvocationID];
    for (int i = 0; i < 10; ++i) {
        tcTextureWeights[gl_InvocationID][i] = vTextureWeights[gl_InvocationID][i];
    }

}