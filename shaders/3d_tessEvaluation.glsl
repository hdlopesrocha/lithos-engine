#version 460 core
#include<structs.glsl>
#include<uniforms.glsl>
layout(triangles, equal_spacing, ccw) in; // Define primitive type and tessellation spacing


in vec3 tcTextureWeights[];
in vec2 tcTextureCoord[];
in vec3 tcPosition[];
in TextureBrush tcProps[];
in uvec3 tcTextureIndices[];
in mat4 tcModel[];
in mat3 tcNormalMatrix[];
in vec3 tcNormal[];


#include<functions.glsl>
#include<triplanar.glsl>

out vec3 teSharpNormal;
out vec2 teTextureCoord;
out vec3 tePosition;
out TextureBrush teProps;
out vec4 teLightViewPosition[SHADOW_MATRIX_COUNT];
flat out uvec3 teTextureIndices;
out mat4 teModel;
uniform TextureBrush overrideProps;
out vec3 teT;
out vec3 teB;
out vec3 teN;
out vec3 teViewDirection;
out vec3 teViewDirectionTangent;
out vec3 teTextureWeights;
out vec3 teBlendFactors;

void main() {
    teProps.textureScale = tcProps[0].textureScale * gl_TessCoord[0] + 
    tcProps[1].textureScale * gl_TessCoord[1] + 
    tcProps[2].textureScale * gl_TessCoord[2];    


    tePosition = gl_TessCoord[0] * tcPosition[0] + gl_TessCoord[1] * tcPosition[1] + gl_TessCoord[2] * tcPosition[2];
    teModel = tcModel[0];
    teTextureWeights = gl_TessCoord;
    teSharpNormal = normalize(cross(tcPosition[1] - tcPosition[0], tcPosition[2] - tcPosition[0]));


    teTextureCoord = tcTextureCoord[0] * gl_TessCoord[0] + tcTextureCoord[1] * gl_TessCoord[1] + tcTextureCoord[2] * gl_TessCoord[2];
    teTextureIndices = tcTextureIndices[0];
   
    if(!billboardEnabled) {
        teTextureCoord *= teProps.textureScale;
    }

    if(!depthEnabled) {


        //vec3 teNormal = tcNormal[0]* gl_TessCoord[0]+tcNormal[1]* gl_TessCoord[1]+tcNormal[2]* gl_TessCoord[2];
        vec3 teNormal = teSharpNormal;

        vec4 t = computeTriplanarTangentVec4(teNormal);
        vec3 iTangent = t.xyz;
        vec3 iBitangent = cross(teNormal, iTangent) * t.w;

        teT = iTangent;
        teB = iBitangent;
        teN = teNormal;
        

        mat3 TBN = mat3(teT,teB, teN);
        teViewDirection = normalize(tePosition-cameraPosition.xyz);
        teViewDirectionTangent = normalize(transpose(TBN) * teViewDirection);
        for(int i = 0; i < SHADOW_MATRIX_COUNT ; ++i ) {
            teLightViewPosition[i] = matrixShadow[i] * vec4(tePosition, 1.0);  
        }



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


            teProps.refractiveIndex = tcProps[0].refractiveIndex * gl_TessCoord[0] + 
                                tcProps[1].refractiveIndex * gl_TessCoord[1] + 
                                tcProps[2].refractiveIndex * gl_TessCoord[2];                                                                                           
        
        }

        

    }


    vec3 normal = abs(teN);
    vec3 blend = pow(abs(normal), vec3(blendSharpness));
    blend /= (blend.x + blend.y + blend.z);
    teBlendFactors = blend;

    gl_Position = viewProjection * vec4(tePosition, 1.0);    

}