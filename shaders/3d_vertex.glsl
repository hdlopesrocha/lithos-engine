#version 460 core
#include<structs.glsl>
#include<uniforms.glsl>

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint brushIndex;     
layout(location = 4) in vec4 inTangent;
layout(location = 5) in mat4 model; 
layout(location = 9) in float shift; 


#include<functions.glsl>


out uint vTextureIndex;
out vec2 vTextureCoord;
out vec3 vPosition;
out vec3 vNormal;
out TextureProperties vProps;
out mat4 vModel;
out vec4 vTangent;  // Tangent.xyz and Handedness.w

uniform TextureProperties brushes[25];
uniform uint brushTextures[25];


void main() {
    vTextureIndex = brushTextures[brushIndex];
    vTextureCoord = textureCoord;
    vTextureCoord.y -= shift;

    vModel = world*model;
    vec3 iPosition = position;
    vec3 iNormal = normal;
    float freq = 1.0/ PI;

    vec3 wPosition = (vModel*vec4(iPosition, 1.0)).xyz;

    if(opacityEnabled) {
        if(iPosition.y > 0.0) {
            iPosition.x += sin(wPosition.x*freq + time);
            iPosition.z += cos(wPosition.z*freq + time);
        }
        iNormal.x += sin(wPosition.x*freq + time);
        iNormal.z += cos(wPosition.z*freq + time);

        iNormal = normalize(iNormal);
    } 

    if(!depthEnabled) {
        vProps = brushes[brushIndex];
        vNormal = iNormal;
    }
    vPosition = (vModel*vec4(iPosition, 1.0)).xyz;
    gl_Position = vec4(vPosition, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(vModel)));
    vec3 T = normalize(normalMatrix * inTangent.xyz);

    // Store transformed tangent
    vTangent = vec4(T, inTangent.w);
}