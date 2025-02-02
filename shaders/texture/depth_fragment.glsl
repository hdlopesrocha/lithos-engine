#version 460 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textureSampler;


float linearizeDepth(float depth) {
    float near = 0.1;
    float far = 512.0;

    float z = depth * 2.0 - 1.0;  // Convert to NDC (-1 to 1)
    float d = (2.0 * near * far) / (far + near - z * (far - near));
    return d/far;


}

void main() {
    float d = texture(textureSampler, TexCoord).r;
    d = linearizeDepth(d);
    FragColor = vec4(d, d ,d ,1.0);
}