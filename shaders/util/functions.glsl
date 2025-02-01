uniform bool overrideTextureEnabled;
uniform uint overrideTexture;

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


vec4 textureBlend(in float ws[20], sampler2DArray ts[20], vec2 uv, int index) {
    if(overrideTextureEnabled) {
        return texture(ts[overrideTexture], vec3(uv, index));
    }
    
    vec4 res = vec4(0.0);
    for(int i=0 ; i < 20; ++i) {
        float w = ws[i];
        if(w>0.0) {
            res += texture(ts[i], vec3(uv, index))*w;
        }
	}
    return res;
}

float floatBlend(in float ws[20], float ts[20]) {
    float res = 0.0;
    for(int i=0 ; i < 20; ++i) {
        float w = ws[i];
        if(w>0.0) {
            res += ts[i]*w;
        }
	}
    return res;
}

