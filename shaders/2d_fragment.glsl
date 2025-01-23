#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D texture1;

void main() {
    vec4 color = texture(texture1, TexCoord);
    FragColor = vec4(color.rgb, 1.0);
}