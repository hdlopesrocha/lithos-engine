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

vec4 textureBlend(in float ws[16], sampler2D ts[16], vec2 uv) {
    vec4 res = vec4(0.0);
    for(int i=0 ; i < 16; ++i) {
        float w = ws[i];
        if(w>0.0) {
            res += texture(ts[i], uv)*w;
        }
	}
    return res;
}

