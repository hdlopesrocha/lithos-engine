#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform float near;
uniform float far;
uniform bool depthEnabled;


float linearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC (-1 to 1)
    return (2.0 * near * far) / (far + near - z * (far - near));
}


void main() {
    if(depthEnabled) {
        float depthValue = texture(texture1, TexCoord).r;
        float linearDepth = linearizeDepth(depthValue) / far;
        FragColor = vec4(vec3(linearDepth), 1.0); 
    } else {
        vec4 color = texture(texture1, TexCoord);
        FragColor = vec4(color.rgb, 1.0);
    }
}