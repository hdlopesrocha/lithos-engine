vec4 computeTriplanarTangentVec4(vec3 normal) {
    vec3 T, B;
    
    if (abs(normal.y) > abs(normal.x) && abs(normal.y) > abs(normal.z)) {
        // Y-dominant plane
        T = vec3(1, 0, 0);
        B = vec3(0, 0, 1);
    } else if (abs(normal.x) > abs(normal.z)) {
        // X-dominant plane
        T = vec3(0, 0, 1);
        B = vec3(0, 1, 0);
    } else {
        // Z-dominant plane
        T = vec3(1, 0, 0);
        B = vec3(0, 1, 0);
    }

    float handedness = (dot(cross(T, B), normal) < 0.0) ? -1.0 : 1.0;
    return vec4(normalize(T), handedness);
}
