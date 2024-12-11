#version 460 core

uniform sampler2D textures[16]; // Texture to sample from
uniform vec3 lightDirection;     // Direction of the light (assumed to be normalized)
uniform uint lightEnabled;

in vec3 oNormal;
in vec2 oTextureCoord;
in float oTextureWeights[16];

out vec4 color;    // Final fragment color

void main() {
    vec3 n = normalize(oNormal);
    float diffuse = max(dot(n, -lightDirection), 0.0);
//    diffuse = clamp(diffuse, 0.5, 1.0);
    if(lightEnabled == 0) {
        diffuse = 1.0;
    }


    vec4 mixedColor = vec4(0.0);
    for(int i=0 ; i < 16; ++i) {
        float w = oTextureWeights[i];
        if(w>0.0) {
            mixedColor += texture(textures[i], oTextureCoord)*w;
        }
	}

    color = vec4(mixedColor.rgb*diffuse, mixedColor.a); 
}