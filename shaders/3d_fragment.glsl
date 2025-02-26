#version 460 core
#include<structs.glsl>

uniform sampler2D shadowMap;
uniform sampler2D noise;
uniform sampler2D depthTexture;
uniform sampler2D underTexture;
uniform sampler2DArray textures[25];

#include<functions.glsl>
#include<functions_fragment.glsl>
#include<parallax.glsl>
#include<depth.glsl>


in vec2 teTextureCoord;
in vec3 teTextureWeights;
flat in uvec3 teTextureIndices;

in TextureProperties teProps;
in vec3 tePosition;
in vec3 teNormal;
in vec4 teLightViewPosition;
in mat4 teModel;

out vec4 color;    // Final fragment color

void main() {
    vec4 positionWorld = teModel * vec4(tePosition, 1.0);
    vec3 position = positionWorld.xyz;    
    mat3 normalMatrix = transpose(inverse(mat3(teModel)));
    vec3 normal = normalize(normalMatrix * teNormal);

    vec2 uv = teTextureCoord;
    if(triplanarEnabled) {
        int plane = triplanarPlane(position, normal);
        uv = triplanarMapping(position, plane, teProps.textureScale) * 0.1;
    }

    if(opacityEnabled) {
        vec4 opacity = textureBlend(teTextureWeights, teTextureIndices, uv, 3);
        if(opacity.r < 0.5) {
            discard;
        }
    }

    float near = 0.1;
    float far = 512.0;
    float currentDepth = linearizeDepth(gl_FragCoord.z, near, far);

    if(depthEnabled) {
        color = vec4(currentDepth/far,0.0,0.0,1.0);
        return;
    }

    vec2 pixelUV = gl_FragCoord.xy / textureSize(depthTexture, 0);
    float existingDepth = linearizeDepth(texture(depthTexture, pixelUV).r, near, far);
    if(existingDepth < currentDepth) {
        discard;
    }

    float effectAmount = sin(time*3.14/4.0)*0.5 + 0.5;
    float distance = length(cameraPosition.xyz - position);
    float distanceFactor = clamp(teProps.parallaxFade / distance, 0.0, 1.0); // Adjust these numbers to fit your scene

    vec3 viewDirection = normalize(position-cameraPosition.xyz);
    mat3 TBN = getTBN(tePosition, teNormal, uv, teModel, false);
    vec3 viewDirectionTangent = normalize(transpose(TBN) * viewDirection);

    if(parallaxEnabled && distanceFactor * teProps.parallaxScale > 0.0) {
       uv = parallaxMapping(teTextureWeights, teTextureIndices, uv, viewDirectionTangent, distanceFactor*teProps.parallaxScale , distanceFactor*teProps.parallaxMinLayers, distanceFactor*teProps.parallaxMaxLayers, int(ceil(distanceFactor*teProps.parallaxRefine)));
    }
  
    vec4 mixedColor = textureBlend(teTextureWeights, teTextureIndices, uv, 0);
    if(mixedColor.a == 0.0) {
        discard;
    }

    vec3 normalMap = textureBlend(teTextureWeights, teTextureIndices, uv, 1).rgb * 2.0 - 1.0;
    normalMap = normalize(normalMap); // Convert to range [-1, 1]
    vec3 worldNormal = normalize(TBN * normalMap);
    if(debugEnabled) {
        color = vec4(visual(worldNormal),1.0);
    } else if(lightEnabled) {
        vec3 specularColor = vec3(1.0,1.0,1.0);
        vec3 reflection = reflect(-lightDirection.xyz, worldNormal);

        float phongSpec = pow(max(dot(reflection, viewDirection), 0.0), teProps.shininess);
        float diffuse = clamp(max(dot(worldNormal, -lightDirection.xyz), 0.0), 0.2, 1.0);

        ShadowProperties shadow; 
        shadow.shadowAmount = 1.0;
        shadow.lightAmount = 1.0;
        if(shadowEnabled) {
            shadow = getShadow(shadowMap, noise, teLightViewPosition, position);
        }

        vec4 refractedColor = vec4(0.0,0.0,0.0,0.0);
        if(teProps.refractiveIndex > 0.0) {
            // Compute refraction
            float eta = 1.0 / teProps.refractiveIndex; // Air to water
            vec3 refractedDir = refract(viewDirectionTangent, normalMap, eta);
            vec2 refractedUV = pixelUV + refractedDir.xy * 0.1; // UV distortion
            refractedColor = texture(underTexture, refractedUV);
        }
        color = refractedColor + vec4(((mixedColor).rgb*diffuse + specularColor * teProps.specularStrength * phongSpec *  shadow.lightAmount)*shadow.shadowAmount , mixedColor.a+teProps.specularStrength * phongSpec *  shadow.lightAmount); 
    }else {
        color = mixedColor;
    }
 }


