#version 460 core

layout(triangles, equal_spacing, ccw) in; // Define primitive type and tessellation spacing
#include<functions.glsl>

uniform sampler2DArray textures[20];
uniform bool debugEnabled;

in float tcTextureWeights[][20];
in vec2 tcTextureCoord[];
in vec3 tcNormal[];
in vec3 tcPosition[];
in TextureProperties tcProps[];


out vec3 teNormal;
out vec2 teTextureCoord;
out float teTextureWeights[20];
out vec3 tePosition;
out TextureProperties teProps;
out vec4 lightViewPosition;

uniform TextureProperties overrideProps;
uniform mat4 modelViewProjection; 
uniform mat4 matrixShadow; 
uniform mat4 model; 


void main() {

    teNormal = normalize(tcNormal[0] * gl_TessCoord[0] + tcNormal[1] * gl_TessCoord[1] + tcNormal[2] * gl_TessCoord[2]);
  
    if(overrideTextureEnabled) {
        teProps = overrideProps;
    } else {
        teProps.parallaxScale = tcProps[0].parallaxScale * gl_TessCoord[0] + 
                                tcProps[1].parallaxScale * gl_TessCoord[1] + 
                                tcProps[2].parallaxScale * gl_TessCoord[2];
        teProps.parallaxMinLayers = tcProps[0].parallaxMinLayers * gl_TessCoord[0] + 
                                    tcProps[1].parallaxMinLayers * gl_TessCoord[1] + 
                                    tcProps[2].parallaxMinLayers * gl_TessCoord[2];
        teProps.parallaxMaxLayers = tcProps[0].parallaxMaxLayers * gl_TessCoord[0] + 
                                    tcProps[1].parallaxMaxLayers * gl_TessCoord[1] + 
                                    tcProps[2].parallaxMaxLayers * gl_TessCoord[2];
        teProps.shininess = tcProps[0].shininess * gl_TessCoord[0] + 
                                    tcProps[1].shininess * gl_TessCoord[1] + 
                                    tcProps[2].shininess * gl_TessCoord[2];
        teProps.specularStrength = tcProps[0].specularStrength * gl_TessCoord[0] + 
                                    tcProps[1].specularStrength * gl_TessCoord[1] + 
                                    tcProps[2].specularStrength * gl_TessCoord[2];
    }


    for (int i = 0; i < 20; ++i) {
        teTextureWeights[i] = (gl_TessCoord[0] * tcTextureWeights[0][i] + gl_TessCoord[1] * tcTextureWeights[1][i] + gl_TessCoord[2] * tcTextureWeights[2][i]);
    }


    // Interpolate the triangle position using barycentric coordinates
    tePosition = gl_TessCoord[0] * tcPosition[0] + gl_TessCoord[1] * tcPosition[1] + gl_TessCoord[2] * tcPosition[2];
    teTextureCoord = tcTextureCoord[0] * gl_TessCoord[0] + tcTextureCoord[1] * gl_TessCoord[1] + tcTextureCoord[2] * gl_TessCoord[2];
    
    if(debugEnabled) {
        tePosition += teNormal*0.02;
    }

    gl_Position = modelViewProjection * vec4(tePosition, 1.0);    
    lightViewPosition = matrixShadow * vec4(tePosition, 1.0);  

}