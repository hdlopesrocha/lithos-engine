


vec4 textureBlend(sampler2DArray ts, vec3 ws, uvec3 ti, vec2 uv) {
    if(overrideEnabled) {
        return texture(ts, vec3(uv, overrideTexture));
    }
    
    vec4 res = vec4(0.0);
    for(int i=0 ; i < 3; ++i) {
        float w = ws[i];
        uint t = ti[i];
        if(w>0.0) {      
            res += texture(ts, vec3(uv, t))*w;
        }
	}
    return res;
}

