#version 460 core

uniform sampler2D textures[10];
uniform sampler2D normalMaps[10];
uniform sampler2D bumpMaps[10];

uniform vec3 lightDirection; 
uniform uint lightEnabled;
uniform vec3 cameraPosition; 
uniform float time;


in vec3 teNormal;
in vec2 teTextureCoord;
in float teTextureWeights[10];
in vec3 tePosition;
in vec3 teTangent;
in vec3 teBitangent;


out vec4 color;    // Final fragment color
uniform uint triplanarEnabled;

#include<functions.glsl>



vec2 parallaxMapping2(vec2 uv, vec3 viewDir) {
      
    int layers = 32;
	const float deltaDepth = 1.0 / float( layers );
	float currentLayerDepth = 0.0;

	vec2 deltaUv = viewDir.xy / float( layers );
	vec2 currentUv = uv;

    float currentDepth = textureBlend(teTextureWeights, bumpMaps, currentUv).r;
    

	for( int i = 0; i < layers; i++ ) {
		if( currentLayerDepth > currentDepth )
			break;

		currentUv += deltaUv;
        currentDepth = textureBlend(teTextureWeights, bumpMaps, currentUv).r;

		currentLayerDepth += deltaDepth;
	}

	vec2 prevUv = currentUv - deltaUv;
    float startDepth = textureBlend(teTextureWeights, bumpMaps, prevUv).r;

	float endDepth = currentDepth - currentLayerDepth;
	float w = endDepth / ( endDepth - startDepth );

	return mix( currentUv, prevUv, w );
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
    float parallaxScale = 0.05;
    float specularStrength = 0.4;
    vec3 specularColor = vec3(1.0,1.0,1.0);


    vec3 normal = normalize(teNormal);
    vec2 uv = teTextureCoord;
    vec3 viewDirection = normalize(tePosition - cameraPosition);


    if(triplanarEnabled == 1) {
        int plane = triplanarPlane(tePosition, teNormal);
        uv = triplanarMapping(tePosition, plane) * 0.1;
    }

    mat3 TBN = getTBN(tePosition, normal, uv);
    vec3 cameraTangent = TBN *  cameraPosition;
    vec3 positionTangent = TBN *  tePosition;
    vec3 viewTangent = normalize(positionTangent - cameraTangent);
    
    uv = parallaxMapping2(uv, viewTangent*parallaxScale);


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

        
        float diffuse = max(dot(worldNormal, -lightDirection), 0.0);



        color = vec4(mixedColor.rgb*diffuse + specularColor * specularStrength * phongSpec, mixedColor.a); 
    }
 }









