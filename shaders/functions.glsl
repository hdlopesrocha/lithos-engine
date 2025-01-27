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

vec2 triplanarMapping(vec3 position, int plane) {
    switch (plane) {
        case 0: return vec2(-position.z, -position.y);
        case 1: return vec2(position.z, -position.y);
        case 2: return vec2(position.x, position.z);
        case 3: return vec2(position.x, -position.z);
        case 4: return vec2(position.x, -position.y);
        case 5: return vec2(-position.x, -position.y);
        default: return vec2(0.0,0.0);
    }
}


vec4 textureBlend(in float ws[10], sampler2D ts[10], vec2 uv) {
    vec4 res = vec4(0.0);
    for(int i=0 ; i < 10; ++i) {
        float w = ws[i];
        if(w>0.0) {
            res += texture(ts[i], uv)*w;
        }
	}
    return res;
}

float floatBlend(in float ws[10], float ts[10]) {
    float res = 0.0;
    for(int i=0 ; i < 10; ++i) {
        float w = ws[i];
        if(w>0.0) {
            res += ts[i]*w;
        }
	}
    return res;
}

struct TextureProperties {
    float parallaxScale;
    float parallaxMinLayers;
    float parallaxMaxLayers;
    float shininess;
    float specularStrength;
};