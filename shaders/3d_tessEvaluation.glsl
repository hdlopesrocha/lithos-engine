#version 460 core
#include<structs.glsl>

layout(triangles, equal_spacing, ccw) in; // Define primitive type and tessellation spacing


in vec3 tcTextureWeights[];
in vec2 tcTextureCoord[];
in vec3 tcNormal[];
in vec3 tcPosition[];
in TextureProperties tcProps[];
in uvec3 tcTextureIndices[];
in mat4 tcModel[];

#include<functions.glsl>

out vec3 teNormal;
out vec2 teTextureCoord;
out vec3 teTextureWeights;
out vec3 tePosition;
out TextureProperties teProps;
out vec4 teLightViewPosition;
flat out uvec3 teTextureIndices;
out mat4 teModel;


uniform TextureProperties overrideProps;


void main() {
    if(!depthEnabled) {
        teNormal = normalize(tcNormal[0] * gl_TessCoord[0] + tcNormal[1] * gl_TessCoord[1] + tcNormal[2] * gl_TessCoord[2]);
    
        if(overrideEnabled) {
            teProps = overrideProps;
        } else {
            teProps.parallaxScale = tcProps[0].parallaxScale * gl_TessCoord[0] + 
                                    tcProps[1].parallaxScale * gl_TessCoord[1] + 
                                    tcProps[2].parallaxScale * gl_TessCoord[2];
            teProps.parallaxMinLayers = tcProps[0].parallaxMinLayers * gl_TessCoord[0] + 
                                        tcProps[1].parallaxMinLayers * gl_TessCoord[1] + 
                                        tcProps[2].parallaxMinLayers * gl_TessCoord[2];
            teProps.parallaxFade = tcProps[0].parallaxFade * gl_TessCoord[0] + 
                                        tcProps[1].parallaxFade * gl_TessCoord[1] + 
                                        tcProps[2].parallaxFade * gl_TessCoord[2];
            teProps.parallaxRefine = tcProps[0].parallaxRefine * gl_TessCoord[0] + 
                                        tcProps[1].parallaxRefine * gl_TessCoord[1] + 
                                        tcProps[2].parallaxRefine * gl_TessCoord[2];

            teProps.parallaxMaxLayers = tcProps[0].parallaxMaxLayers * gl_TessCoord[0] + 
                                        tcProps[1].parallaxMaxLayers * gl_TessCoord[1] + 
                                        tcProps[2].parallaxMaxLayers * gl_TessCoord[2];
            teProps.shininess = tcProps[0].shininess * gl_TessCoord[0] + 
                                        tcProps[1].shininess * gl_TessCoord[1] + 
                                        tcProps[2].shininess * gl_TessCoord[2];
            teProps.specularStrength = tcProps[0].specularStrength * gl_TessCoord[0] + 
                                        tcProps[1].specularStrength * gl_TessCoord[1] + 
                                        tcProps[2].specularStrength * gl_TessCoord[2];
            teProps.textureScale = tcProps[0].textureScale * gl_TessCoord[0] + 
                                tcProps[1].textureScale * gl_TessCoord[1] + 
                                tcProps[2].textureScale * gl_TessCoord[2];    

            teProps.refractiveIndex = tcProps[0].refractiveIndex * gl_TessCoord[0] + 
                                tcProps[1].refractiveIndex * gl_TessCoord[1] + 
                                tcProps[2].refractiveIndex * gl_TessCoord[2];                                                
        }
    }
    teTextureWeights = gl_TessCoord;
    teTextureIndices = tcTextureIndices[0];
    teTextureCoord = tcTextureCoord[0] * gl_TessCoord[0] + tcTextureCoord[1] * gl_TessCoord[1] + tcTextureCoord[2] * gl_TessCoord[2];
    tePosition = gl_TessCoord[0] * tcPosition[0] + gl_TessCoord[1] * tcPosition[1] + gl_TessCoord[2] * tcPosition[2];
    teModel = tcModel[0];

    if(!depthEnabled) {
        teLightViewPosition = matrixShadow * vec4(tePosition, 1.0);  
    }

    gl_Position = viewProjection * vec4(tePosition, 1.0);    

}