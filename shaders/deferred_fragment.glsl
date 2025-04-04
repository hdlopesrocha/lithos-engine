#version 460 core
#include<structs.glsl>


uniform sampler2DArray textures[3];
layout(location = 0) out vec4 FragColor0; // First render target (color)
layout(location = 1) out vec4 FragColor1; // Second render target (normals)
layout(location = 2) out vec4 FragColor2; // Third render target (bump)

uniform bool opacityEnabled;

#include<functions.glsl>
#include<functions_fragment.glsl>

in vec2 gTextureCoord;
in vec3 gTextureWeights;
flat in uvec3 gTextureIndices;
in TextureBrush gProps;
in vec3 gPosition;
in vec3 gNormal;
in vec3 gSharpNormal;

out vec4 color;    // Final fragment color

void main() {
    vec2 uv = gTextureCoord;


    vec4 opacity = textureBlend(textures[2], gTextureIndices, uv, gTextureWeights, gTextureWeights);

    if(opacityEnabled) {
        if(opacity.r < 0.98) {
          //  discard;
        }
    }
    
    FragColor0 = vec4(textureBlend(textures[0], gTextureIndices, uv, gTextureWeights, gTextureWeights).rgb, 1.0);
    FragColor1 = vec4(textureBlend(textures[1], gTextureIndices, uv, gTextureWeights, gTextureWeights).rgb, 1.0);
    FragColor2 = vec4(opacity.rgb, 1.0);

    FragColor0 = vec4(1.0);
    FragColor1 = vec4(1.0);
    FragColor2 = vec4(1.0);
 }


