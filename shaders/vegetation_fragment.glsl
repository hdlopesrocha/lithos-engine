#version 460 core

#include<structs.glsl>

uniform sampler2DArray textures[25];
uniform sampler2D shadowMap;
uniform sampler2D noise;

uniform bool debugEnabled;
uniform bool lightEnabled;
uniform bool shadowEnabled;

uniform vec3 lightDirection; 
uniform vec3 cameraPosition; 
uniform float time;
uniform mat4 model; 

#include<functions.glsl>

in vec2 vTextureCoord;
in TextureProperties vProps;
in vec3 vPosition;
in vec3 vNormal;
in vec4 vLightViewPosition;

out vec4 color;    // Final fragment color

#include<functions_fragment.glsl>


void main() {
    vec2 uv = vTextureCoord;

    vec4 positionWorld = model * vec4(vPosition, 1.0);
    vec3 position = positionWorld.xyz;    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 normal = normalize(normalMatrix * vNormal);

    vec3 viewDirection = normalize(position-cameraPosition);
    mat3 TBN = getTBN(vPosition, vNormal, uv, model, false);
    vec3 viewDirectionTangent = normalize(transpose(TBN) * viewDirection);
  
    vec4 mixedColor = texture(textures[1], vec3(uv, 0));
    vec4 opacity = texture(textures[1], vec3(uv, 3));
    if(opacity.r < 0.1) {
        discard;
    }

    color = mixedColor;
 }

