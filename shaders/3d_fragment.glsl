#version 460 core

#include<structs.glsl>

uniform sampler2DArray textures[20];
uniform sampler2D shadowMap;
uniform sampler2D noise;
uniform sampler2D depthTexture;
uniform sampler2D underTexture;

uniform bool debugEnabled;
uniform bool lightEnabled;
uniform bool shadowEnabled;
uniform bool triplanarEnabled;
uniform bool parallaxEnabled;
uniform bool depthTestEnabled;
uniform float waterRefractiveIndex; // Usually 1.33

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


float linearizeDepth(float depth) {
    float near = 0.1;
    float far = 512.0;

    float z = depth * 2.0 - 1.0;  // Convert to NDC (-1 to 1)
    float d = (2.0 * near * far) / (far + near - z * (far - near));
    return d;
}


void main() {
    vec2 pixelUV = gl_FragCoord.xy / textureSize(underTexture, 0);

    if(depthTestEnabled) {
        float d1 = linearizeDepth(texture(depthTexture, pixelUV).r);
        float d2 = linearizeDepth(gl_FragCoord.z);
        if(d1<d2) {
            discard;
        }
    }


    vec2 uv = teTextureCoord;

    vec4 positionWorld = model * vec4(tePosition, 1.0);
    vec3 position = positionWorld.xyz;    
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 normal = normalize(normalMatrix * teNormal);

    float effectAmount = sin(time*3.14/4.0)*0.5 + 0.5;
    float distance = length(cameraPosition - position);
    float distanceFactor = clamp(teProps.parallaxFade / distance, 0.0, 1.0); // Adjust these numbers to fit your scene

    if(triplanarEnabled) {
        int plane = triplanarPlane(position, normal);
        uv = triplanarMapping(position, plane, teProps.textureScale) * 0.1;
    }

    vec3 viewDirection = normalize(position-cameraPosition);
    mat3 TBN = getTBN(tePosition, teNormal, uv, model, false);
    vec3 viewDirectionTangent = normalize(transpose(TBN) * viewDirection);

    if(parallaxEnabled && distanceFactor * teProps.parallaxScale > 0.0) {
       uv = parallaxMapping(teTextureWeights, uv, viewDirectionTangent, distanceFactor*teProps.parallaxScale , distanceFactor*teProps.parallaxMinLayers, distanceFactor*teProps.parallaxMaxLayers, int(ceil(distanceFactor*teProps.parallaxRefine)));
    }
  
    vec4 mixedColor = textureBlend(teTextureWeights, textures, uv, 0);
    if(mixedColor.a == 0.0) {
        discard;
    }

    if(lightEnabled) {
        vec3 specularColor = vec3(1.0,1.0,1.0);

        vec3 normalMap = textureBlend(teTextureWeights, textures, uv, 1).rgb * 2.0 - 1.0;
        normalMap = normalize(normalMap); // Convert to range [-1, 1]

        vec3 worldNormal = normalize(TBN * normalMap);

        vec3 reflection = reflect(-lightDirection, worldNormal);

        float phongSpec = pow(max(dot(reflection, viewDirection), 0.0), teProps.shininess);
        float diffuse = clamp(max(dot(worldNormal, -lightDirection), 0.0), 0.2, 1.0);

        ShadowProperties shadow; 
        shadow.shadowAmount = 1.0;
        shadow.lightAmount = 1.0;
        if(shadowEnabled) {
            shadow = getShadow(shadowMap, noise, lightViewPosition, position);
        }
        if(debugEnabled) {
            color = vec4(worldNormal,1.0);
        }else {
            vec4 refractedColor = vec4(0.0,0.0,0.0,0.0);

            if(teProps.refractiveIndex > 0.0) {
                // Compute refraction
                float eta = 1.0 / teProps.refractiveIndex; // Air to water
                vec3 refractedDir = refract(viewDirectionTangent, normalMap, eta);
                vec2 refractedUV = pixelUV + refractedDir.xy * 0.1; // UV distortion
                refractedColor = texture(underTexture, refractedUV);
            }

            color = refractedColor + vec4(((mixedColor).rgb*diffuse + specularColor * teProps.specularStrength * phongSpec *  shadow.lightAmount)*shadow.shadowAmount , mixedColor.a+teProps.specularStrength * phongSpec *  shadow.lightAmount); 
        }
    }else {
        if(debugEnabled) {
            color = vec4(visual(normal), 1.0);
        }else {
            color = vec4(1.0,1.0,1.0,1.0);
        }
    }
 }


