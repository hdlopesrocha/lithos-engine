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


vec2 parallaxMapping(vec2 uv, vec3 viewDir, float scale, float minLayers, float maxLayers, int approxCycles) {
    float numLayers = mix(maxLayers, minLayers, dot(vec3(0.0, 0.0, 1.0), -viewDir));  

	float deltaDepth = 1.0 / float( numLayers );
	vec2 deltaUv = (viewDir.xy/viewDir.z) * deltaDepth * scale;
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
        currentHeight = textureBlend(teTextureWeights, textures, currentUv, 2).r;

        if(currentHeight > currentDepth) {
            break;
        }
    }

    for (int i = 0; i < approxCycles; ++i) {
        vec2 midUv = 0.5 * (currentUv + prevUv);
        float midDepth = 0.5 * (currentDepth + prevDepth);
        float midHeight = textureBlend(teTextureWeights, textures, midUv, 2).r;
        
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

bool rotateTBN = false;

mat3 getTBN(vec3 pos, vec3 normal, vec2 uv) {
    vec3 dpdx = dFdx(pos);
    vec3 dpdy = dFdy(pos);
    vec2 dudx = dFdx(uv);
    vec2 dudy = dFdy(uv);

    // Calculate the determinant for the tangent space matrix
    float det = dudx.x * dudy.y - dudx.y * dudy.x;
    float invDet = 1.0 / det;

    // Compute tangent and bitangent vectors
    vec3 tangent = normalize(invDet * (dpdx * dudy.y - dpdy * dudx.y));
    vec3 bitangent = normalize(invDet * (dpdy * dudx.x - dpdx * dudy.x));

    if(rotateTBN) {
        mat3 world = mat3(model); // Extract 3x3 from 4x4
        mat3 normal_matrix = transpose(inverse(world));
        normal = normalize(normal_matrix * normal);
        tangent = normalize(world * tangent);
        bitangent = normalize(world * bitangent);
    }

    // Transform normal map vector to world space
    return mat3(tangent, bitangent, normal);
}

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
    mat3 TBN = getTBN(tePosition, teNormal, uv);
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
            color = vec4((mixedColor.rgb*diffuse + specularColor * teProps.specularStrength * phongSpec * lightPercentage)*finalShadow , mixedColor.a); 
        }
    }else {
        if(debugEnabled) {
            color = vec4(visual(normal), 1.0);
        }else {
            color = vec4(1.0,1.0,1.0,1.0);
        }
    }

 }


