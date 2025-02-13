#version 460 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

#include<structs.glsl>
uniform int layer;

in vec2 teTextureCoord[];
in vec3 teTextureWeights[];
in TextureProperties teProps[];
in vec3 tePosition[];
in vec3 teNormal[];
in vec4 teLightViewPosition[];
in uvec3 teTextureIndices[];

out vec2 gTextureCoord;
out vec3 gTextureWeights;
flat out uvec3 gTextureIndices;

out TextureProperties gProps;
out vec3 gPosition;
out vec3 gNormal;
out vec4 gLightViewPosition;

void main() {
    int layerIndex = 0;

    for (int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;
        gl_Layer = layerIndex; 

        if(layer > 0) {
            gTextureCoord = teTextureCoord[i];
            gTextureWeights = teTextureWeights[i];
            gTextureIndices = teTextureIndices[i];

            gProps = teProps[i];
            gPosition = tePosition[i];
            gNormal = teNormal[i];
            gLightViewPosition = teLightViewPosition[i];
        }

        EmitVertex();
    }

    EndPrimitive();
}