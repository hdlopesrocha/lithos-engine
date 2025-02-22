float linearizeDepth(float depth, float near, float far) {

    float z = depth * 2.0 - 1.0;  // Convert to NDC (-1 to 1)
    float d = (2.0 * near * far) / (far + near - z * (far - near));
    return d;
}
