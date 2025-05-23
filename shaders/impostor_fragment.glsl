#version 460 core
#include<structs.glsl>


uniform sampler2DArray textures[3];


uniform bool opacityEnabled;
uniform bool billboardEnabled; // TODO: set it in ImpostorDrawer
uniform bool overrideEnabled;
uniform uint overrideBrush;

#include<functions.glsl>
#include<functions_fragment.glsl>

in vec2 gTextureCoord;
in vec3 gTextureWeights;
flat in uvec3 gTextureIndices;
in TextureBrush gProps;
in vec3 gPosition;
in vec3 gNormal;
in vec3 gSharpNormal;
in mat4 gModel;
in vec3 gT;
in vec3 gB;
in vec3 gN;

out vec4 color;    // Final fragment color

void main() {
    vec2 uv = gTextureCoord;
    if(billboardEnabled && uv.y < 0.0) {
        discard;
    } 


    if(opacityEnabled) {
        vec4 opacity = textureBlend(textures[2], gTextureIndices, uv, gTextureWeights,gTextureWeights);
        if(opacity.r < 0.98) {
            discard;
        }
    }

    vec3 correctedNormal = gl_FrontFacing ? gNormal : -gNormal;
    mat3 normalMatrix = transpose(inverse(mat3(gModel)));
    vec3 normal = normalize(normalMatrix * correctedNormal);
  
    vec4 mixedColor = textureBlend(textures[0], gTextureIndices, uv, gTextureWeights,gTextureWeights);
    if(mixedColor.a == 0.0) {
        discard;
    }
    mat3 TBN = mat3(normalize(gT),normalize(gB),normalize(gN));
    vec3 normalMap = textureBlend(textures[1], gTextureIndices, uv, gTextureWeights,gTextureWeights).rgb * 2.0 - 1.0;
    normalMap = normalize(normalMap); // Convert to range [-1, 1]
    vec3 worldNormal = normalize(TBN * normalMap);

    //    color = vec4(visual(worldNormal),1.0);
    color = mixedColor;
 }


