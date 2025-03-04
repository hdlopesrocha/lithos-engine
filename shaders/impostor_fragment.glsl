#version 460 core
#include<structs.glsl>

uniform sampler2DArray textures[25];
uniform bool triplanarEnabled;
uniform bool opacityEnabled;
uniform bool overrideEnabled;
uniform uint overrideTexture;

#include<functions.glsl>
#include<functions_fragment.glsl>

in vec2 gTextureCoord;
in vec3 gTextureWeights;
flat in uvec3 gTextureIndices;
in TextureProperties gProps;
in vec3 gPosition;
in vec3 gNormal;
in vec3 gSharpNormal;
in mat4 gModel;

out vec4 color;    // Final fragment color

void main() {
    vec2 uv = gTextureCoord;

    if(triplanarEnabled) {
        int plane = triplanarPlane(gPosition, gSharpNormal);
        uv = triplanarMapping(gPosition, plane, gProps.textureScale) * 0.1;
    }

    if(opacityEnabled) {
        vec4 opacity = textureBlend(textures, gTextureWeights, gTextureIndices, uv, 3);
        if(opacity.r < 0.98) {
            discard;
        }
    }

    vec3 correctedNormal = gl_FrontFacing ? gNormal : -gNormal;
    mat3 normalMatrix = transpose(inverse(mat3(gModel)));
    vec3 normal = normalize(normalMatrix * correctedNormal);

    mat3 TBN = getTBN(gPosition, correctedNormal, uv, gModel, false);
  
    vec4 mixedColor = textureBlend(textures, gTextureWeights, gTextureIndices, uv, 0);
    if(mixedColor.a == 0.0) {
        discard;
    }

    vec3 normalMap = textureBlend(textures, gTextureWeights, gTextureIndices, uv, 1).rgb * 2.0 - 1.0;
    normalMap = normalize(normalMap); // Convert to range [-1, 1]
    vec3 worldNormal = normalize(TBN * normalMap);

    //    color = vec4(visual(worldNormal),1.0);
    color = mixedColor;
 }


