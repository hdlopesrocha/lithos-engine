#version 460 core
#define PI 3.1415926535897932384626433832795

uniform sampler2D textures[10];
uniform sampler2D normalMaps[10];
uniform sampler2D bumpMaps[10];

uniform sampler2D shadowMap;
uniform sampler2D noise;
uniform vec3 lightDirection; 
uniform uint debugEnabled;
uniform uint lightEnabled;
uniform uint triplanarEnabled;
uniform uint parallaxEnabled;
uniform vec3 cameraPosition; 
uniform float time;

#include<functions.glsl>


in vec2 teTextureCoord;
in float teTextureWeights[10];
in vec3 tePosition;
in vec3 teNormal;
in TextureProperties teProps;
in vec4 lightViewPosition;

out vec4 color;    // Final fragment color


vec2 parallaxMapping(vec2 uv, vec3 viewDir, float scale, float minLayers, float maxLayers) {
    float numLayers = mix(maxLayers, minLayers, dot(vec3(0.0, 0.0, 1.0), -viewDir));  

	float deltaDepth = 1.0 / float( numLayers );
	vec2 deltaUv = (viewDir.xy*scale/viewDir.z) * deltaDepth;
	float currentDepth = 1.0;
    vec2 currentUv = uv;
    float currentHeight = 0.0;

    int cycles = 5;
    vec2 prevUv = currentUv;
    float prevDepth = currentDepth;
    float prevHeight = currentHeight;
    float bias = 0.1;
    
    for(int i=0; i < numLayers ; ++i) {
        prevUv = currentUv;
        prevHeight = currentHeight;
        prevDepth = currentDepth;

        currentUv -= deltaUv;
        currentDepth -= deltaDepth;
        currentHeight = textureBlend(teTextureWeights, bumpMaps, currentUv).r;

        if(currentHeight > currentDepth) {
            break;
        }
    }

    for (int i = 0; i < cycles; ++i) {
        vec2 midUv = 0.5 * (currentUv + prevUv);
        float midDepth = 0.5 * (currentDepth + prevDepth);
        float midHeight = textureBlend(teTextureWeights, bumpMaps, midUv).r;
        
        if (midHeight > midDepth) {
            currentUv = midUv;
            currentDepth = midDepth;
            currentHeight = midHeight;
        } else {
            prevUv = midUv;
            prevDepth = midDepth;
            prevHeight = midHeight;
        }
    }
    float delta1 = currentHeight - currentDepth; 
    float delta2 = ( currentDepth + deltaDepth ) - prevHeight; 
    float ratio = delta1/(delta1+delta2);
    return mix(currentUv, prevUv, ratio);
}

mat3 getTBN(vec3 pos, vec3 normal, vec2 uv) {
    vec3 dpdx = dFdx(pos);
    vec3 dpdy = dFdy(pos);
    vec2 dUVdx = dFdx(uv);
    vec2 dUVdy = dFdy(uv);

    // Calculate the determinant for the tangent space matrix
    float det = dUVdx.x * dUVdy.y - dUVdx.y * dUVdy.x;
    float invDet = 1.0 / det;

    // Compute tangent and bitangent vectors
    vec3 tangent = normalize(invDet * (dpdx * dUVdy.y - dpdy * dUVdx.y));
    vec3 bitangent = normalize(invDet * (dpdy * dUVdx.x - dpdx * dUVdy.x));

    tangent = normalize(tangent - dot(tangent, normal) * normal);
    bitangent = normalize(cross(tangent, normal));

    // Transform normal map vector to world space
    return mat3(tangent, bitangent, normal);
}

vec3 visual(vec3 v) {
    return v*0.5 + vec3(0.5);
}


void main() {
    float effectAmount = sin(time*3.14/4.0)*0.5 + 0.5;
   
    vec3 normal = normalize(teNormal);
    vec2 uv = teTextureCoord;
    vec3 viewDirection = normalize(tePosition - cameraPosition);

    if(triplanarEnabled == 1) {
        int plane = triplanarPlane(tePosition, normal);
        uv = triplanarMapping(tePosition, plane) * 0.1;
    }

    mat3 TBN = getTBN(tePosition, normal, uv);
    vec3 viewTangent = normalize(transpose(TBN) * viewDirection);
    

    if(parallaxEnabled == 1 && teProps.parallaxScale > 0.0) {
       uv = parallaxMapping(uv, viewTangent, teProps.parallaxScale , teProps.parallaxMinLayers, teProps.parallaxMaxLayers);
    }
  
    vec4 mixedColor = textureBlend(teTextureWeights, textures, uv);
    if(mixedColor.a == 0.0) {
        discard;
    }

    if(debugEnabled == 1) {
        color = vec4(1.0,1.0,1.0,1.0);
    } else if(lightEnabled == 0) {
        color = mixedColor; 
    } else {
        float specularStrength = 0.4;
        vec3 specularColor = vec3(1.0,1.0,1.0);

        vec3 normalMap = textureBlend(teTextureWeights, normalMaps, uv).xyz;
        normalMap = normalize(normalMap * 2.0 - 1.0); // Convert to range [-1, 1]

        vec3 worldNormal = normalize(TBN * normalMap);

        vec3 reflection = reflect(-lightDirection, worldNormal);
        float phongSpec = pow(max(dot(reflection, viewDirection), 0.0), teProps.shininess);
        float diffuse = clamp(max(dot(worldNormal, -lightDirection), 0.0), 0.2, 1.0);

        vec3 shadowPosition = lightViewPosition.xyz / lightViewPosition.w; 
        float bias = 0.002;
        float shadow = texture(shadowMap, shadowPosition.xy).r < shadowPosition.z-bias ? 0.0 : 1.0;;
        float texelSize = 1.0/4098.0;

        vec2 noiseCoords = (tePosition.xy+ tePosition.z)* PI;
        float sumShadow = shadow;


        int blurRadius = 4;
        int totalSamples = 1;
        int maxSamples = 5;


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
        float lightPercentage = sumShadow/totalSamples;

        float finalShadow = (1.0 - shadowAlpha) + lightPercentage*shadowAlpha;

        color = vec4((mixedColor.rgb*diffuse + specularColor * specularStrength * phongSpec * lightPercentage)*finalShadow , mixedColor.a); 
    }

 }


