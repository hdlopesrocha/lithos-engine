#version 460 core
#include<structs.glsl>
#include<uniforms.glsl>

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint brushIndex;     
layout(location = 4) in float shift; 
layout(location = 5) in mat4 model; 


#include<functions.glsl>

uniform uint brushTextures[32];
uniform TextureBrush brushes[32];

out uint vTextureIndex;
out vec2 vTextureCoord;
out vec3 vPosition;
out TextureBrush vProps;
out mat4 vModel;
out vec3 vT;
out vec3 vB;
out vec3 vN;


#include<triplanar.glsl>

void main() {
    vTextureIndex = brushTextures[brushIndex];
    vTextureCoord = textureCoord;
    vTextureCoord.y -= shift;

    vModel = world*model;

    float freq = 1.0/ PI;

    vec3 wPosition = (vModel*vec4(position, 1.0)).xyz;
 
    mat3 normalMatrix = transpose(inverse(mat3(vModel)));
    vN = normalize(normalMatrix * normal);
    vProps = brushes[brushIndex];


    if(!depthEnabled) {
        vec4 t = computeTriplanarTangentVec4(vN);
        vec3 iTangent = t.xyz;
        vec3 iBitangent = cross(vN, iTangent) * t.w;

        vT = iTangent;
        vB = iBitangent;
    }
    if(billboardEnabled) {
        if(position.y > 0.0) {
            wPosition.x += sin(wPosition.x*freq + time);
            wPosition.z += cos(wPosition.z*freq + time);
        }
        vN.x += sin(wPosition.x*freq + time);
        vN.z += cos(wPosition.z*freq + time);

        vN = normalize(vN);
    } 


    vPosition = wPosition;
    gl_Position = vec4(vPosition, 1.0);

}