#version 460 core
#define PI 3.1415926535897932384626433832795

uniform sampler2DArray textures[20];
uniform sampler2D shadowMap;
uniform sampler2D noise;

uniform bool debugEnabled;
uniform bool lightEnabled;
uniform bool shadowEnabled;
uniform bool triplanarEnabled;
uniform bool parallaxEnabled;

uniform vec3 lightDirection; 
uniform vec3 cameraPosition; 
uniform float time;
uniform mat4 model; 

#include<functions.glsl>

in vec2 teTextureCoord;
in float teTextureWeights[20];
in TextureProperties teProps;
in vec3 tePosition;
in vec3 teNormal;
in vec4 lightViewPosition;

out vec4 color;    // Final fragment color

#include<functions_fragment.glsl>


vec3 visual(vec3 v) {
    return v*0.5 + vec3(0.5);
}


void main() {
    vec2 uv = teTextureCoord;

    vec4 positionWorld = model * vec4(tePosition, 1.0);
    vec3 position = positionWorld.xyz;    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 normal = normalize(normalMatrix * teNormal);

    float effectAmount = sin(time*3.14/4.0)*0.5 + 0.5;
    float distance = length(cameraPosition - position);
    float distanceFactor = clamp(32.0 / distance, 0.0, 1.0); // Adjust these numbers to fit your scene

    if(triplanarEnabled) {
        int plane = triplanarPlane(position, normal);
        uv = triplanarMapping(position, plane, teProps.textureScale) * 0.1;
    }

    vec3 viewDirection = normalize(position - cameraPosition);
    mat3 TBN = getTBN(tePosition, teNormal, uv, model, false);
    vec3 viewDirectionTangent = normalize(transpose(TBN) * viewDirection);

    if(parallaxEnabled && distanceFactor * teProps.parallaxScale > 0.0) {
       uv = parallaxMapping(uv, viewDirectionTangent, distanceFactor*teProps.parallaxScale , distanceFactor*teProps.parallaxMinLayers, distanceFactor*teProps.parallaxMaxLayers, int(ceil(distanceFactor*5.0)));
    }
  
    vec4 mixedColor = textureBlend(teTextureWeights, textures, uv, 0);
    if(mixedColor.a == 0.0) {
        discard;
    }

    if(lightEnabled) {
        vec3 specularColor = vec3(1.0,1.0,1.0);

        vec3 normalMap = textureBlend(teTextureWeights, textures, uv, 1).xyz;
        normalMap = normalize(normalMap * 2.0 - 1.0); // Convert to range [-1, 1]

        vec3 worldNormal = normalize(TBN * normalMap);

        vec3 reflection = reflect(-lightDirection, worldNormal);
        float phongSpec = pow(max(dot(reflection, viewDirectionTangent), 0.0), teProps.shininess);
        float diffuse = clamp(max(dot(worldNormal, -lightDirection), 0.0), 0.2, 1.0);

        float finalShadow = 1.0;
        float lightPercentage = 1.0;
        if(shadowEnabled) {
            vec3 shadowPosition = lightViewPosition.xyz / lightViewPosition.w; 
            float bias = 0.002;
            float shadow = texture(shadowMap, shadowPosition.xy).r < shadowPosition.z-bias ? 0.0 : 1.0;
            float texelSize = 1.0/4098.0;

            vec2 noiseCoords = (position.xy)* PI;
            float sumShadow = shadow;


            int blurRadius = 20;
            int totalSamples = 1;
            int maxSamples = 8;


            for(int radius = blurRadius; radius > 0; --radius) {
                for(int samples=0; samples < maxSamples ; ++samples) {

                    vec4 noiseSample = texture(noise, noiseCoords);
                    float sAngle = noiseSample.r * 2.0 * PI;
                    float sRadius = radius;
                    float sX = sRadius * cos(sAngle);
                    float sY = sRadius * sin(sAngle);
                    
                    vec2 shadowUV = shadowPosition.xy+vec2(sX,sY)*texelSize;
                    float shadowValue = texture(shadowMap, shadowUV).r;
                    
                    sumShadow += shadowValue < shadowPosition.z-bias ? 0.0 : 1.0;
                    ++totalSamples;
                    
                    noiseCoords += noiseSample.xy;
                }
                if(sumShadow == totalSamples || sumShadow == 0) {
                    break;
                }
            }


            float shadowAlpha = 0.6;
            lightPercentage = sumShadow/totalSamples;
            finalShadow = (1.0 - shadowAlpha) + lightPercentage*shadowAlpha;
        }
        if(debugEnabled) {
            color = vec4(visual(worldNormal), 1.0);
        }else {
            color = vec4((mixedColor.rgb*diffuse + specularColor * teProps.specularStrength * phongSpec * lightPercentage)*finalShadow , mixedColor.a+teProps.specularStrength * phongSpec * lightPercentage); 
        }
    }else {
        if(debugEnabled) {
            color = vec4(visual(normal), 1.0);
        }else {
            color = vec4(1.0,1.0,1.0,1.0);
        }
    }

 }


