#version 460 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

#include<structs.glsl>

in vec2 teTextureCoord[];
in float teTextureWeights[][20];
in TextureProperties teProps[];
in vec3 tePosition[];
in vec3 teNormal[];
in vec4 teLightViewPosition[];

out vec2 gTextureCoord;
out float gTextureWeights[20];
out TextureProperties gProps;
out vec3 gPosition;
out vec3 gNormal;
out vec4 gLightViewPosition;

void main() {
    int layerIndex = 0;

    for (int i = 0; i < 3; ++i) {
        gl_Position = gl_in[i].gl_Position;
        gl_Layer = layerIndex; 

        gTextureCoord = teTextureCoord[i];
        for(int j =0; j < 20 ; ++j) {
            gTextureWeights[j] = teTextureWeights[i][j];
        }
        gProps = teProps[i];
        gPosition = tePosition[i];
        gNormal = teNormal[i];
        gLightViewPosition = teLightViewPosition[i];


        EmitVertex();
    }

    EndPrimitive();
}