#version 460 core

#include<structs.glsl>
#include<uniforms.glsl>

out vec4 color;    // Final fragment color
in float distance;
in float scale;
in vec4 vertexColor;     


void main() {
    float logDepth = log2(1.0 + gl_FragCoord.z) / log2(far + 1.0);
    gl_FragDepth = logDepth;

    float r =  abs(distance/scale);
    float range = 0.2;
    if(r > range) {
        discard;
    }
    color = distance >=0.0 ? vertexColor : vec4(1.0,1.0,1.0,1.0);

    color.a =0.2*(1.0 - abs(r)/range);
    //color = vec4(1.0,1.0,1.0, 0.2);
}


