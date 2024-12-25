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
    vec3 n = normalize(teNormal);
    float diffuse = max(dot(n, -lightDirection), 0.0);
    if(lightEnabled == 0) {
        diffuse = 1.0;
    }

    vec2 uv = teTextureCoord;
    if(triplanarEnabled == 1) {
        uv = triplanarMapping(tePosition, n, 0.1);
    }


    vec4 mixedColor = vec4(0.0);
    for(int i=0 ; i < 16; ++i) {
        float w = teTextureWeights[i];
        if(w>0.0) {
            mixedColor += texture(textures[i], uv)*w;
        }
	}
    if(mixedColor.a == 0.0) {
        discard;
    }

    color = vec4(mixedColor.rgb*diffuse, mixedColor.a); 
 
 }