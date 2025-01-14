#version 460 core

uniform sampler2D textures[10];
uniform sampler2D normalMaps[10];
uniform sampler2D bumpMaps[10];


uniform vec3 lightDirection; 
uniform uint lightEnabled;
uniform vec3 cameraPosition; 
uniform float time;


in vec2 teTextureCoord;
in float teTextureWeights[10];
in vec3 tePosition;
in vec3 teNormal;
in float teParallaxScale;
in float teParallaxMinLayers;
in float teParallaxMaxLayers;

out vec4 color;    // Final fragment color
uniform uint triplanarEnabled;
uniform uint parallaxEnabled;

#include<functions.glsl>

vec2 parallaxMapping(vec2 uv, vec3 viewDir, float scale, float minLayers, float maxLayers) {
    float numLayers = mix(maxLayers, minLayers, clamp(dot(vec3(0.0, 0.0, -1.0), viewDir), 0.0, 1.0));  

	float deltaDepth = 1.0 / float( numLayers );
	vec2 deltaUv = (viewDir.xy*scale/viewDir.z) * deltaDepth;

	float currentDepth = 1.0;
    vec2 currentUv = uv;
    float currentHeight = 0.0;
    int cycles = 1;

    while(cycles > 0) {
        for(int i=0; i < numLayers ; ++i) {
            currentUv -= deltaUv;
            currentDepth -= deltaDepth;
            currentHeight = textureBlend(teTextureWeights, bumpMaps, currentUv).r;

            if(currentDepth < currentHeight) {
                break;
            }
        }
        --cycles;
        if(cycles > 0) {
            currentUv += deltaUv;
            currentDepth += deltaDepth;
            currentHeight = textureBlend(teTextureWeights, bumpMaps, currentUv).r;
            deltaUv /= numLayers;
            deltaDepth /= numLayers;
        }
    }

    
    vec2 prevUv = currentUv + deltaUv;
    float prevHeight = textureBlend(teTextureWeights, bumpMaps, prevUv).r;

 
    float weight = (currentHeight - currentDepth) / (currentHeight  -prevHeight + deltaDepth);
    return mix(currentUv, prevUv, weight);
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
    float effectAmount = sin(time*3.14/4.0);
    float shininess = 32.0;
    float specularStrength = 0.4;
    vec3 specularColor = vec3(1.0,1.0,1.0);

    vec3 normal = normalize(teNormal);
    vec2 uv = teTextureCoord;
    vec3 viewDirection = normalize(tePosition - cameraPosition);

    if(triplanarEnabled == 1) {
        int plane = triplanarPlane(tePosition, normal);
        uv = triplanarMapping(tePosition, plane) * 0.1;
    }

    mat3 TBN = getTBN(tePosition, normal, uv);
    vec3 viewTangent = normalize(transpose(TBN) * viewDirection);
    

    
    if(teParallaxScale > 0.0) {
       uv = parallaxMapping(uv, -viewTangent, teParallaxScale, teParallaxMinLayers, teParallaxMaxLayers);
    }
    vec3 normalMap = textureBlend(teTextureWeights, normalMaps, uv).xyz;
    normalMap = normalize(normalMap * 2.0 - 1.0); // Convert to range [-1, 1]

    vec3 worldNormal = normalize(TBN * normalMap);

    vec3 reflection = reflect(-lightDirection, worldNormal);
    float phongSpec = pow(max(dot(reflection, viewDirection), 0.0), shininess);

    vec4 mixedColor = textureBlend(teTextureWeights, textures, uv);
    if(mixedColor.a == 0.0) {
        discard;
    }

    if(lightEnabled == 0) {
        color = mixedColor; 
    }
    else {
        float diffuse = clamp(max(dot(worldNormal, -lightDirection), 0.0), 0.2, 1.0);
        color = vec4(mixedColor.rgb*diffuse + specularColor * specularStrength * phongSpec, mixedColor.a); 
    }

   // color = vec4(visual(normal), 1.0);
 }









