#version 460 core

uniform sampler2D textures[16];
uniform sampler2D normalMaps[16];
uniform sampler2D bumpMaps[16];

uniform vec3 lightDirection;     // Direction of the light (assumed to be normalized)
uniform uint lightEnabled;

in vec3 teNormal;
in vec2 teTextureCoord;
in float teTextureWeights[16];
in vec3 tePosition;



out vec4 color;    // Final fragment color
uniform uint triplanarEnabled;

#include<functions.glsl>

void main() {
    vec3 normal = normalize(teNormal);

    vec2 uv = teTextureCoord;
    
    float diffuse = max(dot(normal, -lightDirection), 0.0);
    if(lightEnabled == 0) {
        diffuse = 1.0;
    }

    vec4 mixedColor = textureBlend(teTextureWeights, textures, uv);
    if(mixedColor.a == 0.0) {
        discard;
    }

    color = vec4(mixedColor.rgb*diffuse, mixedColor.a); 
 
 }