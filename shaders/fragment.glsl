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



out vec4 color;    // Final fragment color
uniform uint triplanarEnabled;

#include<functions.glsl>

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir, float scale) {
    // Number of layers and step size
    const int numLayers = 32;
    const float layerDepth = 1.0 / float(numLayers);

    // Initial values
    float currentDepth = 0.0;
    vec2 currentTexCoords = texCoords;

    // Depth from the height map
    float heightFromMap;

    // Step size along the view direction
    vec2 deltaTexCoords = viewDir.xy * scale / viewDir.z / float(numLayers);

    // Iterative search
    for (int i = 0; i < numLayers; ++i) {
        heightFromMap = textureBlend(teTextureWeights, bumpMaps, currentTexCoords).r;

        if (currentDepth > heightFromMap) {
            break;
        }

        currentTexCoords -= deltaTexCoords;
        currentDepth += layerDepth;
    }

    // Optionally refine with binary search
    return currentTexCoords;
}

void main() {
    vec3 normal = normalize(teNormal);
    vec2 uv = teTextureCoord;



    // Compute derivatives of position and texture coordinates
    vec3 dpdx = dFdx(tePosition);
    vec3 dpdy = dFdy(tePosition);
    vec2 dtdx = dFdx(uv);
    vec2 dtdy = dFdy(uv);

    // Solve for tangent and bitangent
    float determinant = dtdx.x * dtdy.y - dtdx.y * dtdy.x;
    vec3 tangent = normalize( (dtdy.y * dpdx - dtdx.y * dpdy) / determinant);
    vec3 bitangent = normalize( (-dtdy.x * dpdx + dtdx.x * dpdy) / determinant);


    // Construct the TBN matrix
    mat3 TBN = mat3(normalize(tangent),
                    normalize(bitangent),
                    normalize(normal));


    
    vec3 viewDirWorld = normalize(cameraPosition - tePosition);
    vec3 viewDirTangent = normalize(TBN * viewDirWorld);

    
    float diffuse = max(dot(normal, -lightDirection), 0.0);
    if(lightEnabled == 0) {
        diffuse = 1.0;
    }

    uv = parallaxMapping(uv, viewDirTangent, 0.1);


    vec4 mixedColor = textureBlend(teTextureWeights, textures, uv);
    if(mixedColor.a == 0.0) {
        discard;
    }

    color = vec4(mixedColor.rgb*diffuse, mixedColor.a); 
 
 }