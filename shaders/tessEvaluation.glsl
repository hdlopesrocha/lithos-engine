#version 460 core

layout(triangles, equal_spacing, ccw) in; // Define primitive type and tessellation spacing

uniform sampler2D textures[10];
uniform sampler2D normalMaps[10];
uniform sampler2D bumpMaps[10];

in float tcTextureWeights[][10];
in vec2 tcTextureCoord[];
in vec3 tcNormal[];
in vec3 tcPosition[];

out vec3 teNormal;
out vec2 teTextureCoord;
out float teTextureWeights[10];
out vec3 tePosition;
out vec3 teTangent;
out vec3 teBitangent;


uniform mat4 model;      // Model transformation matrix
uniform mat4 view;       // View transformation matrix
uniform mat4 projection; // Projection transformation matrix
uniform uint triplanarEnabled;


#include<functions.glsl>

void main() {
   // Interpolate attributes using barycentric coordinates
    teNormal = tcNormal[0] * gl_TessCoord[0] + tcNormal[1] * gl_TessCoord[1] + tcNormal[2] * gl_TessCoord[2];
    teNormal = normalize(mat3(transpose(inverse(model))) * teNormal);


    
    for (int i = 0; i < 10; ++i) {
        teTextureWeights[i] = (gl_TessCoord[0] * tcTextureWeights[0][i] + gl_TessCoord[1] * tcTextureWeights[1][i] + gl_TessCoord[2] * tcTextureWeights[2][i]);
    }


    // Interpolate the triangle position using barycentric coordinates
    tePosition = gl_TessCoord[0] * tcPosition[0] + gl_TessCoord[1] * tcPosition[1] + gl_TessCoord[2] * tcPosition[2];
    teTextureCoord = tcTextureCoord[0] * gl_TessCoord[0] + tcTextureCoord[1] * gl_TessCoord[1] + tcTextureCoord[2] * gl_TessCoord[2];
    

    //float height = textureBlend(teTextureWeights, bumpMaps, teTextureCoord).r;


    gl_Position = projection * view * model * vec4(tePosition, 1.0);    

}