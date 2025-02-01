#version 460 core

#include<structs.glsl>

uniform sampler2DArray textures[20];
uniform sampler2D shadowMap;
uniform sampler2D noise;
uniform sampler2D depthTexture;

uniform bool debugEnabled;
uniform bool lightEnabled;
uniform bool shadowEnabled;
uniform bool triplanarEnabled;
uniform bool parallaxEnabled;
uniform bool depthTestEnabled;

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
#include<parallax.glsl>




void main() {
    if(depthTestEnabled) {
        float d1 = texture(depthTexture, gl_FragCoord.xy).r;
        float d2 = gl_FragCoord.z;
        if(d1>d2) {
         //   discard;
        }

    }


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
       uv = parallaxMapping(teTextureWeights, uv, viewDirectionTangent, distanceFactor*teProps.parallaxScale , distanceFactor*teProps.parallaxMinLayers, distanceFactor*teProps.parallaxMaxLayers, int(ceil(distanceFactor*5.0)));
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

        ShadowProperties shadow; 
        shadow.shadowAmount = 1.0;
        shadow.lightAmount = 1.0;
        if(shadowEnabled) {
            shadow = getShadow(shadowMap, noise, lightViewPosition, position);
        }
        if(debugEnabled) {

            float d = gl_FragCoord.z;


            color = vec4(visual(worldNormal), 1.0);
            color = vec4(d,d,d,1.0);
        }else {
            color = vec4((mixedColor.rgb*diffuse + specularColor * teProps.specularStrength * phongSpec *  shadow.lightAmount)*shadow.shadowAmount , mixedColor.a+teProps.specularStrength * phongSpec *  shadow.lightAmount); 
        }
    }else {
        if(debugEnabled) {
            color = vec4(visual(normal), 1.0);
        }else {
            color = vec4(1.0,1.0,1.0,1.0);
        }
    }

 }


