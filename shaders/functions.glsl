vec2 triplanarMapping(vec3 position, vec3 normal, float scale) {
    vec3 absNormal = abs(normal);
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z) {
        return position.yz * scale;
    } else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
        return position.zx * scale;
    } else {
        return position.xy * scale;
    }
}
