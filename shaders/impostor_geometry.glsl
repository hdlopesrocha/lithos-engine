#version 460 core
#include<structs.glsl>

in vec3 vPosition[];
in vec3 vNormal[];
in vec2 vTextureCoord[];
in uint vTextureIndex[];
in mat4 vModel[];
in TextureProperties vProps[];

out uint gTextureIndex;
out vec2 gTextureCoord;
out vec3 gTextureWeights;
out vec3 gPosition;
out vec3 gNormal;
out TextureProperties gProps; 
out mat4 gModel;
out vec3 gSharpNormal;
flat out uvec3 gTextureIndices;
out vec3 gT;
out vec3 gB;
out vec3 gN;

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

#include<triplanar.glsl>

void main() {
    for (int i = 0; i < 3; ++i) {
        gTextureIndices[i] = vTextureIndex[i];
    }
    gSharpNormal = normalize(cross(vPosition[1] - vPosition[0], vPosition[2] - vPosition[0]));



    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            gTextureWeights[j] = i == j ? 1.0 : 0.0;
        }

        gTextureIndex = vTextureIndex[i];
        gTextureCoord = vTextureCoord[i];
        gPosition = vPosition[i];
        gNormal = vNormal[i];
        gProps = vProps[i];
        gModel = vModel[i];

        mat3 normalMatrix = transpose(inverse(mat3(gModel)));
        gN = normalize(normalMatrix * gNormal);

        vec4 t = computeTriplanarTangentVec4(gN);
        vec3 iTangent = t.xyz;
        vec3 iBitangent = cross(gN, iTangent) * t.w;

        gT = iTangent;
        gB = iBitangent;

        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }

    // End the primitive
    EndPrimitive();
}
