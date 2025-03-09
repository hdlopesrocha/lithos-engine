#version 460 core
#include<structs.glsl>
#include<uniforms.glsl>
layout(triangles, equal_spacing, ccw) in; // Define primitive type and tessellation spacing


in vec3 tcTextureWeights[];
in vec2 tcTextureCoord[];
in vec3 tcNormal[];
in vec3 tcPosition[];
in TextureProperties tcProps[];
in uvec3 tcTextureIndices[];
in mat4 tcModel[];
in mat3 tcNormalMatrix[];
in vec3 tcT[];
in vec3 tcB[];
in vec3 tcN[];

#include<functions.glsl>

out vec3 teSharpNormal;
out vec2 teTextureCoord;
out vec3 teTextureWeights;
out vec3 tePosition;
out TextureProperties teProps;
out vec4 teLightViewPosition[SHADOW_MATRIX_COUNT];
flat out uvec3 teTextureIndices;
out mat4 teModel;
uniform TextureProperties overrideProps;
out vec3 teT;
out vec3 teB;
out vec3 teN;

void main() {
    if(!depthEnabled) {
        teSharpNormal = normalize(cross(tcPosition[1] - tcPosition[0], tcPosition[2] - tcPosition[0]));

        if(overrideEnabled) {
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
            teProps.parallaxFade = tcProps[0].parallaxFade * gl_TessCoord[0] + 
                                        tcProps[1].parallaxFade * gl_TessCoord[1] + 
                                        tcProps[2].parallaxFade * gl_TessCoord[2];
            teProps.parallaxRefine = tcProps[0].parallaxRefine * gl_TessCoord[0] + 
                                        tcProps[1].parallaxRefine * gl_TessCoord[1] + 
                                        tcProps[2].parallaxRefine * gl_TessCoord[2];

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

    if(triplanarEnabled) {
        int plane = triplanarPlane(tePosition, teSharpNormal);
        teTextureCoord = triplanarMapping(tePosition, plane, teProps.textureScale) * 0.1;
    }

    if(!depthEnabled) {
        
        // Output TBN vectors
        teT = tcT[0]* gl_TessCoord[0]+tcT[1]* gl_TessCoord[1]+tcT[2]* gl_TessCoord[2];
        teB = tcB[0]* gl_TessCoord[0]+tcB[1]* gl_TessCoord[1]+tcB[2]* gl_TessCoord[2];
        teN = tcN[0]* gl_TessCoord[0]+tcN[1]* gl_TessCoord[1]+tcN[2]* gl_TessCoord[2];
        
        for(int i = 0; i < SHADOW_MATRIX_COUNT ; ++i ) {
            teLightViewPosition[i] = matrixShadow[i] * vec4(tePosition, 1.0);  
        }
    }

    gl_Position = viewProjection * vec4(tePosition, 1.0);    

}