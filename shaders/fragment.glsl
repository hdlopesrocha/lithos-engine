#version 460 core

uniform sampler2D textures[16]; // Texture to sample from
uniform vec3 lightDirection;     // Direction of the light (assumed to be normalized)
uniform uint lightEnabled;

in vec3 teNormal;
in vec2 teTextureCoord;
in float teTextureWeights[16];
in vec3 tePosition;

out vec4 color;    // Final fragment color
uniform uint triplanarEnabled;

void main() {
    vec3 n = normalize(teNormal);
    float diffuse = max(dot(n, -lightDirection), 0.0);
    if(lightEnabled == 0) {
        diffuse = 1.0;
    }

    float scale = 0.1;
    vec2 uv = teTextureCoord;
    if(triplanarEnabled == 1) {
        vec3 absNormal = abs(n);
        // Determine the dominant axis
        if (absNormal.x > absNormal.y && absNormal.x > absNormal.z) {
            uv = tePosition.yz * scale;
        } else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
            uv = tePosition.zx * scale;
        } else {
            uv = tePosition.xy * scale;
        }
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