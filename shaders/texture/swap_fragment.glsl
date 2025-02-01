#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textureSampler;
uniform float near;
uniform float far;

float linearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // Back to NDC (-1 to 1)
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main() {
    vec4 color = texture(textureSampler, TexCoord);
    FragColor = color;
}