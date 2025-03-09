#version 460 core
#include<structs.glsl>
#include<uniforms.glsl>

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint brushIndex;     
layout(location = 5) in float shift; 
layout(location = 6) in mat4 model; 


#include<functions.glsl>


out uint vTextureIndex;
out vec2 vTextureCoord;
out vec3 vPosition;
out vec3 vNormal;
out TextureProperties vProps;
out mat4 vModel;
out vec4 vTangent;  // Tangent.xyz and Handedness.w
out vec3 vT;
out vec3 vB;
out vec3 vN;

uniform TextureProperties brushes[25];
uniform uint brushTextures[25];

vec4 computeTriplanarTangentVec4(vec3 normal) {
    vec3 T, B;
    
    if (abs(normal.y) > abs(normal.x) && abs(normal.y) > abs(normal.z)) {
        // Y-dominant plane
        T = vec3(1, 0, 0);
        B = vec3(0, 0, 1);
    } else if (abs(normal.x) > abs(normal.z)) {
        // X-dominant plane
        T = vec3(0, 0, 1);
        B = vec3(0, 1, 0);
    } else {
        // Z-dominant plane
        T = vec3(1, 0, 0);
        B = vec3(0, 1, 0);
    }

    float handedness = (dot(cross(T, B), normal) < 0.0) ? -1.0 : 1.0;
    return vec4(normalize(T), handedness);
}

void main() {
    vTextureIndex = brushTextures[brushIndex];
    vTextureCoord = textureCoord;
    vTextureCoord.y -= shift;

    vModel = world*model;
    vec3 iPosition = position;
    mat3 normalMatrix = transpose(inverse(mat3(vModel)));
    vec3 iNormal = normalize(normalMatrix * normal);
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
    vec4 t = computeTriplanarTangentVec4(iNormal);
    vec3 iTangent = t.xyz;
    vec3 iBitangent = cross(iNormal, iTangent) * t.w;

    vT = iTangent;
    vB = iBitangent;
    vN = iNormal;

    // Store transformed tangent
   //vtan = computeTriplanarTangentVec4(iNormal);
}