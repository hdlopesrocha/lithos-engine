#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textureSampler;


float linearizeDepth(float depth) {
    float near = 0.1;
    float far = 512.0;

    return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));

}

void main() {
    float d = texture(textureSampler, TexCoord).r;
    d = linearizeDepth(d);
    FragColor = vec4(d, d ,d ,1.0);
}