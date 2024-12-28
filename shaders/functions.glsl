int triplanarPlane(vec3 position, vec3 normal) {
    vec3 absNormal = abs(normal);
    if (absNormal.x > absNormal.y && absNormal.x > absNormal.z) {
        return 0;
    } else if (absNormal.y > absNormal.x && absNormal.y > absNormal.z) {
        return 1;
    } else {
        return 2;
    }
}

vec2 triplanarMapping(vec3 position, int plane) {
    if (plane == 0) {
        return position.yz;
    } else if (plane == 1) {
        return position.zx;
    } else {
        return position.xy;
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

