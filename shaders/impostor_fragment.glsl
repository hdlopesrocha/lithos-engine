#version 460 core
#include<structs.glsl>

uniform sampler2DArray textures[25];

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
    if(uv.y < 0.0) {
        discard;
    } 

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

    float effectAmount = sin(time*3.14/4.0)*0.5 + 0.5;
    float distance = length(cameraPosition.xyz - gPosition);
    float distanceFactor = clamp(gProps.parallaxFade / distance, 0.0, 1.0); // Adjust these numbers to fit your scene

    vec3 viewDirection = normalize(gPosition-cameraPosition.xyz);
    mat3 TBN = getTBN(gPosition, correctedNormal, uv, gModel, false);
    vec3 viewDirectionTangent = normalize(transpose(TBN) * viewDirection);
  
    vec4 mixedColor = textureBlend(textures, gTextureWeights, gTextureIndices, uv, 0);
    if(mixedColor.a == 0.0) {
        discard;
    }

    vec3 normalMap = textureBlend(textures, gTextureWeights, gTextureIndices, uv, 1).rgb * 2.0 - 1.0;
    normalMap = normalize(normalMap); // Convert to range [-1, 1]
    vec3 worldNormal = normalize(TBN * normalMap);

    if(debugEnabled) {
        color = vec4(visual(worldNormal),1.0);
    } else {
        color = mixedColor;
    }
 }


