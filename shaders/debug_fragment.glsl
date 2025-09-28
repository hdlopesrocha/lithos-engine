#version 460 core

#include<structs.glsl>
#include<uniforms.glsl>

out vec4 color;    // Final fragment color
in float distance;
in float scale;
void main() {
    float logDepth = log2(1.0 + gl_FragCoord.z) / log2(far + 1.0);
    gl_FragDepth = logDepth;

    float r =  abs(distance/scale);
    if(r > 100.2) {
        discard;
    }
    color = vec4(1.0,1.0,1.0, 0.2);
}


