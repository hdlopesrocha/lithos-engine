#version 460 core
#define PI 3.1415926535897932384626433832795

#include<structs.glsl>

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

in vec2 vTextureCoord;
in float vTextureWeights[20];
in TextureProperties vProps;
in vec3 vPosition;
in vec3 vNormal;
in vec4 lightViewPosition;

out vec4 color;    // Final fragment color

#include<functions_fragment.glsl>


vec3 visual(vec3 v) {
    return v*0.5 + vec3(0.5);
}


void main() {
    vec2 uv = vTextureCoord;

    vec4 positionWorld = model * vec4(vPosition, 1.0);
    vec3 position = positionWorld.xyz;    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 normal = normalize(normalMatrix * vNormal);

    float effectAmount = sin(time*3.14/4.0)*0.5 + 0.5;
    float distance = length(cameraPosition - position);
    float distanceFactor = clamp(32.0 / distance, 0.0, 1.0); // Adjust these numbers to fit your scene

    if(triplanarEnabled) {
        int plane = triplanarPlane(position, normal);
        uv = triplanarMapping(position, plane, vProps.textureScale) * 0.1;
    }

    vec3 viewDirection = normalize(position - cameraPosition);
    mat3 TBN = getTBN(vPosition, vNormal, uv, model, false);
    vec3 viewDirectionTangent = normalize(transpose(TBN) * viewDirection);

    if(parallaxEnabled && distanceFactor * vProps.parallaxScale > 0.0) {
       uv = parallaxMapping(vTextureWeights,uv, viewDirectionTangent, distanceFactor*vProps.parallaxScale , distanceFactor*vProps.parallaxMinLayers, distanceFactor*vProps.parallaxMaxLayers, int(ceil(distanceFactor*5.0)));
    }
  
    vec4 mixedColor = textureBlend(vTextureWeights, textures, uv, 0);
    if(mixedColor.a == 0.0) {
        discard;
    }

    if(lightEnabled) {
        vec3 specularColor = vec3(1.0,1.0,1.0);

        vec3 normalMap = textureBlend(vTextureWeights, textures, uv, 1).xyz;
        normalMap = normalize(normalMap * 2.0 - 1.0); // Convert to range [-1, 1]

        vec3 worldNormal = normalize(TBN * normalMap);

        vec3 reflection = reflect(-lightDirection, worldNormal);
        float phongSpec = pow(max(dot(reflection, viewDirectionTangent), 0.0), vProps.shininess);
        float diffuse = clamp(max(dot(worldNormal, -lightDirection), 0.0), 0.2, 1.0);

        ShadowProperties shadow; 
        shadow.shadowAmount = 1.0;
        shadow.lightAmount = 1.0;
        if(shadowEnabled) {
            shadow = getShadow(shadowMap, lightViewPosition, position);
        }
        if(debugEnabled) {
            color = vec4(visual(worldNormal), 1.0);
        }else {
            color = vec4((mixedColor.rgb*diffuse + specularColor * vProps.specularStrength * phongSpec *  shadow.lightAmount)*shadow.shadowAmount , mixedColor.a+vProps.specularStrength * phongSpec *  shadow.lightAmount); 
        }
    }else {
        if(debugEnabled) {
            color = vec4(visual(normal), 1.0);
        }else {
            color = vec4(1.0,1.0,1.0,1.0);
        }
    }

 }


