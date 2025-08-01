#version 460 core
#include<structs.glsl>
#include<uniforms.glsl>

// Input from the vertex array
layout(location = 0) in vec4 position; 
layout(location = 1) in vec4 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in int brushIndex;     
layout(location = 4) in mat4 model; 
layout(location = 8) in float shift; 
layout(location = 9) in uint animation; 


#include<functions.glsl>

uniform uint brushTextures[32];
uniform TextureBrush brushes[32];

out uint vTextureIndex;
out vec2 vTextureCoord;
out vec3 vPosition;
out TextureBrush vProps;
out mat4 vModel;
out vec3 vNormal;



void main() {
    vTextureCoord = textureCoord;
    vTextureCoord.y -= shift;

    vModel = world*model;
    vPosition = (vModel*vec4(position.xyz, 1.0)).xyz;



 
    mat3 normalMatrix = transpose(inverse(mat3(vModel)));
    vNormal = normalize(normalMatrix * normal.xyz);
    
    if(overrideEnabled){
        vTextureIndex = brushTextures[overrideBrush];
    }else {
        vTextureIndex = brushTextures[uint(brushIndex)];
        vProps = brushes[uint(brushIndex)];
    }


    if(billboardEnabled) {
        float freq = 0.02/ PI;
        vec3 wPosition = vPosition;
        if(position.y > 0.0) {
            vPosition.x += sin(wPosition.x*freq + time);
            vPosition.z += cos(wPosition.z*freq + time);
        }
        vNormal.x += sin(vPosition.x*freq + time);
        vNormal.z += cos(vPosition.z*freq + time);

        vNormal = normalize(vNormal);
    } 
    
    gl_Position = vec4(vPosition, 1.0);

}