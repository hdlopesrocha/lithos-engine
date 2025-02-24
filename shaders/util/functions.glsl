
int triplanarPlane(vec3 position, vec3 normal) {
    vec3 absNormal = abs(normal);
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z) {
        return normal.x > 0 ? 0 : 1;
    } else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
        return normal.y > 0 ? 2 : 3;
    } else {
        return normal.z > 0 ? 4 : 5;
    }
}

vec2 triplanarMapping(vec3 position, int plane, vec2 scale) {
    switch (plane) {
        case 0: return vec2(-position.z, -position.y)*scale;
        case 1: return vec2(position.z, -position.y)*scale;
        case 2: return vec2(position.x, position.z)*scale;
        case 3: return vec2(position.x, -position.z)*scale;
        case 4: return vec2(position.x, -position.y)*scale;
        case 5: return vec2(-position.x, -position.y)*scale;
        default: return vec2(0.0,0.0);
    }
}

vec3 visual(vec3 v) {
    return v*0.5 + vec3(0.5);
}
