


vec3 visual(vec3 v) {
    return v*0.5 + vec3(0.5);
}

// Function to convert HSV to RGB
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 brushColor(uint i) {
    float hue = fract(float(i) * 0.61803398875); // Golden ratio ensures a good spread
    return hsv2rgb(vec3(hue, 0.7, 0.9)); // Convert from HSV to RGB with fixed saturation & brightness
}
