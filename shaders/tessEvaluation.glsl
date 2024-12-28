#version 460 core

layout(triangles, equal_spacing, ccw) in; // Define primitive type and tessellation spacing


uniform sampler2D textures[16];
uniform sampler2D normalMaps[16];
uniform sampler2D bumpMaps[16];

in float tcTextureWeights[][16];
in vec2 tcTextureCoord[];
in vec3 tcNormal[];
in vec3 tcPosition[];

out vec3 teNormal;
out vec2 teTextureCoord;
out float teTextureWeights[16];
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
    teNormal = tcNormal[0] * gl_TessCoord.x + tcNormal[1] * gl_TessCoord.y + tcNormal[2] * gl_TessCoord.z;
    teNormal = normalize(mat3(transpose(inverse(model))) * teNormal);




    teTextureCoord = tcTextureCoord[0] * gl_TessCoord.x + tcTextureCoord[1] * gl_TessCoord.y + tcTextureCoord[2] * gl_TessCoord.z;
    
    for (int i = 0; i < 16; ++i) {
        teTextureWeights[i] = (gl_TessCoord.x * tcTextureWeights[0][i] + gl_TessCoord.y * tcTextureWeights[1][i] + gl_TessCoord.z * tcTextureWeights[2][i]);
    }


    // Interpolate the triangle position using barycentric coordinates
    tePosition = gl_TessCoord.x * tcPosition[0] +
                  gl_TessCoord.y * tcPosition[1] +
                  gl_TessCoord.z * tcPosition[2];

    vec2 uv = teTextureCoord;
    if(triplanarEnabled == 1) {
        uv = triplanarMapping(tePosition, teNormal, 0.1);
    }




    gl_Position = projection * view * model * vec4(tePosition, 1.0);     
}