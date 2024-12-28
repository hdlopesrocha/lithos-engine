#version 460 core

uniform sampler2D textures[16];
uniform sampler2D normalMaps[16];
uniform sampler2D bumpMaps[16];

uniform vec3 lightDirection; 
uniform uint lightEnabled;
uniform vec3 cameraPosition; 


in vec3 teNormal;
in vec2 teTextureCoord;
in float teTextureWeights[16];
in vec3 tePosition;
in vec3 teTangent;
in vec3 teBitangent;


out vec4 color;    // Final fragment color
uniform uint triplanarEnabled;

#include<functions.glsl>



vec2 parallaxMapping2(vec2 uv, vec2 displacement, float pivot ) {
    int layers = 8;
	const float layerDepth = 1.0 / float( layers );
	float currentLayerDepth = 0.0;

	vec2 deltaUv = displacement / float( layers );
	vec2 currentUv = uv + pivot * displacement;

    float currentDepth = textureBlend(teTextureWeights, bumpMaps, currentUv).r;


	for( int i = 0; i < layers; i++ ) {
		if( currentLayerDepth > currentDepth )
			break;

		currentUv -= deltaUv;
        currentDepth = textureBlend(teTextureWeights, bumpMaps, currentUv).r;

		currentLayerDepth += layerDepth;
	}

	vec2 prevUv = currentUv + deltaUv;
	float endDepth = currentDepth - currentLayerDepth;
    float startDepth = textureBlend(teTextureWeights, bumpMaps, prevUv).r;

	float w = endDepth / ( endDepth - startDepth );

	return mix( currentUv, prevUv, w );
}



void main() {
    vec3 normal = normalize(teNormal);
    vec2 uv = teTextureCoord;



    if(triplanarEnabled == 1) {
        int plane = triplanarPlane(tePosition, teNormal);
        uv = triplanarMapping(tePosition, plane) * 0.1;
    }


    vec3 dpdx = dFdx(tePosition);
    vec3 dpdy = dFdy(tePosition);
    vec2 dUVdx = dFdx(uv);
    vec2 dUVdy = dFdy(uv);

    // Calculate the determinant for the tangent space matrix
    float det = dUVdx.x * dUVdy.y - dUVdx.y * dUVdy.x;
    float invDet = 1.0 / det;

    // Compute tangent and bitangent vectors
    vec3 tangent = normalize(invDet * (dpdx * dUVdy.y - dpdy * dUVdx.y));
    vec3 bitangent = normalize(invDet * (dpdy * dUVdx.x - dpdx * dUVdy.x));


    vec3 normalMap = textureBlend(teTextureWeights, normalMaps, uv).xyz;
    normalMap = normalize(normalMap * 2.0 - 1.0); // Convert to range [-1, 1]


    // Transform normal map vector to world space
    mat3 TBN = mat3(tangent, bitangent, teNormal);
    vec3 worldNormal = normalize(TBN * normalMap);





    float diffuse = max(dot(worldNormal, -lightDirection), 0.0);
    if(lightEnabled == 0) {
        diffuse = 1.0;
    }

    vec4 mixedColor = textureBlend(teTextureWeights, textures, uv);
    if(mixedColor.a == 0.0) {
        discard;
    }

    color = vec4(mixedColor.rgb*diffuse, mixedColor.a); 
 
 }









