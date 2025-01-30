#version 460 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2DArray baseTexture;
uniform sampler2DArray overlayTexture;

uniform float perlinScale;
uniform float perlinTime;
uniform int perlinIterations;
uniform float brightness;
uniform float contrast;
in flat int Layer;

#include<perlin.glsl>

void main() {
    float scale = perlinScale;
    float time = perlinTime;
    
    float weight = 1.0;
    float totalWeight = 0.0;
    float noise = 0.0;

    for(int i=0 ; i < perlinIterations; ++i) {
        float p = perlin(TexCoord, scale, time)* weight;
        noise += p;
        totalWeight += weight;
        weight *= 0.5;
        scale *= 2.0;
    }

    float factor = noise/totalWeight;
    factor = clamp((factor - 0.5) * contrast + 0.5 + brightness, 0.0, 1.0);


    vec4 baseColor = texture(baseTexture, vec3(TexCoord,Layer));
    vec4 overlayColor = texture(overlayTexture, vec3(TexCoord,Layer));

    vec4 color = baseColor*(factor)  + overlayColor*(1.0-factor);

    FragColor = vec4(color.rgb, 1.0);
    //FragColor = vec4(vec3(factor), 1.0);
}