#version 460 core

#include<structs.glsl>

uniform sampler2DArray textures[25];
uniform sampler2D shadowMap;
uniform sampler2D noise;

uniform bool debugEnabled;
uniform bool lightEnabled;
uniform bool shadowEnabled;

uniform vec3 lightDirection; 
uniform vec3 cameraPosition; 
uniform float time;
uniform mat4 model; 

#include<functions.glsl>

in vec2 vTextureCoord;
in TextureProperties vProps;
in vec3 vPosition;
in vec3 vNormal;
in vec4 vLightViewPosition;

out vec4 color;    // Final fragment color

#include<functions_fragment.glsl>


void main() {
    vec2 uv = vTextureCoord;

    vec4 positionWorld = model * vec4(vPosition, 1.0);
    vec3 position = positionWorld.xyz;    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 normal = normalize(normalMatrix * vNormal);

    vec3 viewDirection = normalize(position-cameraPosition);
    mat3 TBN = getTBN(vPosition, vNormal, uv, model, false);
    vec3 viewDirectionTangent = normalize(transpose(TBN) * viewDirection);
  
    vec4 mixedColor = texture(textures[0], vec3(uv, 0));
    if(mixedColor.a == 0.0) {
      //  discard;
    }

    if(lightEnabled) {
        vec3 specularColor = vec3(1.0,1.0,1.0);

        vec3 normalMap = texture(textures[0], vec3(uv, 1)).rgb * 2.0 - 1.0;
        normalMap = normalize(normalMap); // Convert to range [-1, 1]

        vec3 worldNormal = normalize(TBN * normalMap);

        vec3 reflection = reflect(-lightDirection, worldNormal);

        float phongSpec = pow(max(dot(reflection, viewDirection), 0.0), vProps.shininess);
        float diffuse = clamp(max(dot(worldNormal, -lightDirection), 0.0), 0.2, 1.0);

        ShadowProperties shadow; 
        shadow.shadowAmount = 1.0;
        shadow.lightAmount = 1.0;
        if(shadowEnabled) {
            shadow = getShadow(shadowMap, noise, vLightViewPosition, position);
        }
        if(debugEnabled) {
            color = vec4(worldNormal,1.0);
        }else {
            color = vec4(((mixedColor).rgb*diffuse + specularColor * vProps.specularStrength * phongSpec *  shadow.lightAmount)*shadow.shadowAmount , mixedColor.a+vProps.specularStrength * phongSpec *  shadow.lightAmount); 
        }
    }else {
        if(debugEnabled) {
            color = vec4(visual(normal), 1.0);
        }else {
            color = vec4(1.0,1.0,1.0,1.0);
        }
    }
                color = vec4(visual(vPosition), 1.0);

 }

