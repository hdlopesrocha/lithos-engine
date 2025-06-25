#version 460 core

#include<structs.glsl>
#include<uniforms.glsl>

out vec4 color;    // Final fragment color
in float distance;

void main() {
    float logDepth = log2(1.0 + gl_FragCoord.z) / log2(far + 1.0);
    gl_FragDepth = logDepth;
    float scale = 128.0;
    color = vec4(distance < 0.0 ? 1.0 - abs(distance/scale) : 0.0 ,distance >= 0.0 ? 1.0 - abs(distance/scale) : 0.0 ,0.0,1.0);
}


