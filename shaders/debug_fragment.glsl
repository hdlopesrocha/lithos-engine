#version 460 core

#include<structs.glsl>
#include<uniforms.glsl>

uniform sampler2D sampler;
out vec4 color;    // Final fragment color
in float distance;
in float scale;
in vec4 vertexColor;     
in float brush;
in vec2 texCoord;



void main() {
    float logDepth = log2(1.0 + gl_FragCoord.z) / log2(far + 1.0);
    gl_FragDepth = logDepth;


    if(brush == 0.0) {
        vec4 c = texture(sampler, texCoord);
        if(c.a <= 0.1) {
            discard;
        }
        color = c;    
    }
    else if(true) {
        float r =  abs(distance/scale);
        float range = 0.2;
        if(r > range) {
            discard;
        }
        color = distance >=0.0 ? vertexColor : vec4(1.0,1.0,1.0,1.0);
        color.a =0.2*(1.0 - abs(r)/range);
    } else if(false) {
        color = texture(sampler, texCoord) * vertexColor;
        if(color.a <= 0.1) {
            discard;
        }
    }

}


