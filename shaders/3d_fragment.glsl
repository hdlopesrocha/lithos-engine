#version 460 core
#include<structs.glsl>
#include<uniforms.glsl>

uniform sampler2D shadowMap[SHADOW_MATRIX_COUNT];
uniform sampler2D noise;
uniform sampler2D depthTexture;
uniform sampler2D underTexture;
uniform sampler2DArray textures[3];

#include<functions.glsl>
#include<functions_fragment.glsl>
#include<shadow.glsl>
#include<parallax.glsl>
#include<depth.glsl>


in vec2 teTextureCoord;
in vec3 teTextureWeights;
flat in uvec3 teTextureIndices;

in TextureProperties teProps;
in vec3 tePosition;
in vec3 teSharpNormal;
in vec4 teLightViewPosition[SHADOW_MATRIX_COUNT];
in mat4 teModel;
in vec3 teT;
in vec3 teB;
in vec3 teN;

out vec4 color;    // Final fragment color


void main() {


    vec2 uv = teTextureCoord;
    if(billboardEnabled && uv.y < 0.0) {
        discard;
    } 

    if(opacityEnabled) {
        vec4 opacity = textureBlend(textures[2], teTextureWeights, teTextureIndices, uv);
        if(opacity.r < 0.98) {
            discard;
        }
    }

    if(depthEnabled) {
        return;
    }

    vec2 pixelUV = gl_FragCoord.xy / textureSize(depthTexture, 0);
    float currentDepth = gl_FragCoord.z;
    float existingDepth = texture(depthTexture, pixelUV).r;
    if(existingDepth < currentDepth) {
        discard;
    }


    float effectAmount = sin(time*3.14/4.0)*0.5 + 0.5;
    float distance = length(cameraPosition.xyz - tePosition);
    float distanceFactor = clamp(teProps.parallaxFade / distance, 0.0, 1.0); // Adjust these numbers to fit your scene


    mat3 TBN = mat3(normalize(teT), normalize(teB), normalize(teN));

   //color = vec4(visual(teT),1.0);
   //return;

    vec3 viewDirection = normalize(tePosition-cameraPosition.xyz);
    vec3 viewDirectionTangent = normalize(transpose(TBN) * viewDirection);

    if(parallaxEnabled && distanceFactor * teProps.parallaxScale > 0.0) {
       uv = parallaxMapping(textures[2], teTextureWeights, teTextureIndices, uv, viewDirectionTangent, distanceFactor*teProps.parallaxScale , distanceFactor*teProps.parallaxMinLayers, distanceFactor*teProps.parallaxMaxLayers, int(ceil(distanceFactor*teProps.parallaxRefine)));
    }
  
    vec4 mixedColor = textureBlend(textures[0], teTextureWeights, teTextureIndices, uv);
    if(mixedColor.a == 0.0) {
        discard;
    }

    vec3 normalMap = textureBlend(textures[1], teTextureWeights, teTextureIndices, uv).rgb * 2.0 - 1.0;
    normalMap = normalize(normalMap); // Convert to range [-1, 1]
    vec3 worldNormal = normalize(TBN * normalMap);

    if(debugEnabled) {
        color = textureBlend(textures[2], teTextureWeights, teTextureIndices, uv);
    } else if(lightEnabled) {
        vec3 specularColor = vec3(1.0,1.0,1.0);
        vec3 reflection = reflect(-lightDirection.xyz, worldNormal);

        float phongSpec = pow(max(dot(reflection, viewDirection), 0.0), teProps.shininess);
        float diffuse = clamp(max(dot(worldNormal, -lightDirection.xyz), 0.0), 0.2, 1.0);

        ShadowProperties shadow; 
        shadow.shadowAmount = 1.0;
        shadow.lightAmount = 1.0;
        if(shadowEnabled) {
            shadow = getShadow(shadowMap, noise, teLightViewPosition, tePosition, teSharpNormal);
        }
        

        vec4 refractedColor = vec4(0.0,0.0,0.0,0.0);
        if(teProps.refractiveIndex > 0.0) {
            // Compute refraction

            float eta = 1.0 / teProps.refractiveIndex; // Air to water
            vec3 refractedDir = refract(viewDirectionTangent, normalMap, eta);
            vec2 refractedUV = pixelUV + refractedDir.xy * 0.1; // UV distortion
        
            float d2 = texture(depthTexture, refractedUV).r;

            refractedColor = texture(underTexture, currentDepth < d2 ? refractedUV : pixelUV);
        }

        color = refractedColor + vec4((mixedColor.rgb*diffuse + specularColor * teProps.specularStrength * phongSpec *  shadow.lightAmount)*shadow.shadowAmount , mixedColor.a+teProps.specularStrength * phongSpec *  shadow.lightAmount); 

    }else {
        color = mixedColor;
    }
 }


