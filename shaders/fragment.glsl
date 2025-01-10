#version 460 core

uniform sampler2D textures[10];
uniform sampler2D normalMaps[10];
uniform sampler2D bumpMaps[10];
uniform float parallaxScale[10];


uniform vec3 lightDirection; 
uniform uint lightEnabled;
uniform vec3 cameraPosition; 
uniform float time;


in vec3 teNormal;
in vec2 teTextureCoord;
in float teTextureWeights[10];
in vec3 tePosition;


out vec4 color;    // Final fragment color
uniform uint triplanarEnabled;

#include<functions.glsl>

vec2 parallaxMapping(vec2 uv, vec3 viewDir, float scale) {
    const float minLayers = 8.0;
    const float maxLayers = 32.0;
    float numLayers = mix(maxLayers, minLayers, max(dot(vec3(0.0, 0.0, 1.0), viewDir), 0.0));  

	const float deltaDepth = 1.0 / float( numLayers );
	float currentLayerDepth = 0.0;
	vec2 deltaUv = (viewDir.xy*scale) / float( numLayers );
	    
    vec2 currentUv = uv;
    float currentDepth = 1.0 - textureBlend(teTextureWeights, bumpMaps, currentUv).r;

	for(int i=0; i < numLayers ; ++i) {
        if(currentLayerDepth > currentDepth) {
            break;
        }
		currentUv -= deltaUv;
        currentDepth = 1.0 - textureBlend(teTextureWeights, bumpMaps, currentUv).r;
		currentLayerDepth += deltaDepth;
	}

    vec2 prevUv = currentUv + deltaUv;

    float afterDepth  = currentDepth - currentLayerDepth;
    float beforeDepth = 1.0 - textureBlend(teTextureWeights, bumpMaps, prevUv).r - currentLayerDepth + deltaDepth;
    
    float weight = afterDepth / (afterDepth - beforeDepth);
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

    // Transform normal map vector to world space
    return mat3(tangent, bitangent, normal);
}

void main() {
    float effectAmount = sin(time*3.14)*0.5+ 0.5;
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
    vec3 viewTangent = normalize(TBN * viewDirection);
    
    float scale = floatBlend(teTextureWeights, parallaxScale);
    uv = parallaxMapping(uv, viewTangent, scale);

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
 }









