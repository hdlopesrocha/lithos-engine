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
in vec3 tePosition;
in vec4 teLightViewPosition[SHADOW_MATRIX_COUNT];
in mat4 teModel;
in vec3 teTangent;
in vec3 teBitangent;
in vec3 teNormal;
in vec3 teSharpNormal;
in vec3 teTextureWeights;
in vec3 teBlendFactors;
flat in uvec3 teTextureIndices;
in TextureBrush teProps;

out vec4 color;    // Final fragment color



void main() {

    vec2 uv = teTextureCoord;
    float parallaxDepth = 0.0;

      // Sample the blend factor from texture[2]
    float opacityFactor = textureBlend(textures[2], teTextureIndices, uv, teTextureWeights, teBlendFactors).r;

    if(billboardEnabled) {
        // Check if both uv.x and uv.y are within [0, 1]
        if(uv.y < 0.0 || uv.y > 1.0 || uv.x < 0.0 || uv.x > 1.0) {
           discard;  
        } 
    }

    // Determine whether to keep the fragment based on opacity and blend factor
    if(opacityEnabled) {
        // Also keep if the blend factor is high enough
        if(opacityFactor < 0.98) {
            discard;  
        }
    }

 
    // Get the current fragment depth from built-in variable
    float currentDepth = gl_FragCoord.z;


    // If depth writing is enabled, write the current depth and stop further processing.
    if(depthEnabled) {
        gl_FragDepth = currentDepth;
        return; // Stops further calculations but writes depth
    }

    // Otherwise, compare with existing depth from a depth texture.
    vec2 pixelUV = gl_FragCoord.xy / textureSize(depthTexture, 0);
    float existingDepth = texture(depthTexture, pixelUV).r;
    if(existingDepth < currentDepth) {
        discard;
    }

    //mat3 TBN = mat3(normalize(teTangent), normalize(teBitangent), normalize(teNormal));
    vec3 correctedNormal = gl_FrontFacing ? teNormal : -teNormal;
    mat3 TBN = getTBN(tePosition, debugMode == 16 ? teNormal: correctedNormal, uv, teModel, false);

    float effectAmount = sin(time*3.14/4.0)*0.5 + 0.5;
    float distance = length(cameraPosition.xyz - tePosition);
    float distanceFactor = 1.0 - clamp(distance/parallaxDistance, 0.0, 1.0); 
    distanceFactor = pow(distanceFactor, parallaxPower);
    distanceFactor = clamp(distanceFactor, 0.0, 1.0);


    vec3 viewDirection = normalize(tePosition-cameraPosition.xyz);
    vec3 viewDirectionTangent = normalize(transpose(TBN) * viewDirection);

    if(parallaxEnabled && distanceFactor * teProps.parallaxScale > 0.0) {
       vec3 parallaxedUv = parallaxMapping(
            textures[2], 
            teTextureIndices, 
            uv, 
            viewDirectionTangent, 
            teProps,
            teTextureWeights,
            teBlendFactors,
            distanceFactor
        );
        parallaxDepth = length(abs(parallaxedUv.xy - uv));
        uv = parallaxedUv.xy;
    }
  
    vec4 mixedColor = textureBlend(textures[0], teTextureIndices, uv, teTextureWeights, teBlendFactors);
    if(mixedColor.a == 0.0) {
        discard;
    }
   

    vec3 normalMap = textureBlend(textures[1], teTextureIndices, uv, teTextureWeights, teBlendFactors).rgb * 2.0 - 1.0;
    normalMap = normalize(normalMap); // Convert to range [-1, 1]
    vec3 worldNormal = normalize(TBN * normalMap);

    
    if(lightEnabled) {
        vec3 specularColor = vec3(1.0,1.0,1.0);
        vec3 reflection = reflect(-lightDirection.xyz, worldNormal);

        float phongSpec = pow(max(dot(reflection, viewDirection), 0.0), teProps.shininess);
        float diffuse = clamp(max(dot(worldNormal, -lightDirection.xyz), 0.0), 0.2, 1.0);

        float shadowAmount = 1.0;
        float shadowAlpha = 0.6;
        if(shadowEnabled) {
            shadowAmount = getShadow(shadowMap, noise, teLightViewPosition, tePosition, teSharpNormal);
        }
        shadowAmount = shadowAmount*(1.0-shadowAlpha)+shadowAlpha; 

        vec4 refractedColor = vec4(0.0,0.0,0.0,0.0);
        if(teProps.refractiveIndex > 0.0) {
            // Compute refraction

            float eta = 1.0 / teProps.refractiveIndex; // Air to water
            vec3 refractedDir = refract(viewDirectionTangent, normalMap, eta);
            vec2 refractedUV = pixelUV + refractedDir.xy * 0.1; // UV distortion
        
            float d2 = texture(depthTexture, refractedUV).r;

            refractedColor = texture(underTexture, currentDepth < d2 ? refractedUV : pixelUV);
        }

        color = refractedColor + vec4((mixedColor.rgb*diffuse + specularColor * teProps.specularStrength * phongSpec)*shadowAmount , mixedColor.a+teProps.specularStrength * phongSpec); 

    }else {
        color = mixedColor;
    }



    if(debugEnabled) {
        if(debugMode == 0){
            color = textureBlend(textures[0], teTextureIndices, uv, teTextureWeights, teBlendFactors);
        }
        else if(debugMode == 1){
            color = textureBlend(textures[1], teTextureIndices, uv, teTextureWeights, teBlendFactors);
        }
        else if(debugMode == 2){
            color = vec4(vec3(opacityFactor), 1.0);
        }
        else if(debugMode == 3) {
            color = vec4(mod(uv,vec2(1.0)),1.0,1.0);
        }
        else if(debugMode == 4) {
            color = vec4(visual(TBN[0]),1.0);
        }
        else if(debugMode == 5) {
            color = vec4(visual(TBN[1]),1.0);
        }
        else if(debugMode == 6) {
            color = vec4(visual(TBN[2]),1.0);
        } 
        else if(debugMode == 7) {
            color = vec4(visual(teSharpNormal),1.0);
        }
        else if(debugMode == 8) {
            color = vec4(visual(worldNormal),1.0);
        }
        else if(debugMode == 9) {
            color = vec4(visual(normalMap),1.0);
        }
        else if(debugMode == 10) {
            float near = 0.1;
            float far = 1024.0;
            color = vec4(vec3(linearizeDepth(currentDepth, near, far)/far),1.0);
        }
        else if(debugMode == 11) {
            color = vec4(teBlendFactors,1.0);
        }
        else if(debugMode == 12) {
            color = vec4(teTextureWeights,1.0);
        }
        else if(debugMode == 13) {
            color = vec4(vec3(distanceFactor),1.0);
        }      
        else if(debugMode == 14) {
            color = vec4(vec3(abs(parallaxDepth)),1.0);
        }
        else if(debugMode == 15) {
            if(dot(-viewDirection, teNormal) < 0.1) {
                color = vec4(0.0,0.0,0.0,1.0);
            }
        }
        else if(debugMode == 17) {


            vec3 c =    brushColor(teTextureIndices.x)*teTextureWeights.x +
                        brushColor(teTextureIndices.y)*teTextureWeights.y +
                        brushColor(teTextureIndices.z)*teTextureWeights.z;

            // Generate colors using bitwise operations

            color = vec4(c.r,c.g,c.b,1.0);    
        }
        return;
    }
 }


