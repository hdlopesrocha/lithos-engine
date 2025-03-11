


vec4 textureBlend(sampler2DArray ts, uvec3 textureIndices, vec2 uv, vec3 textureWeights, vec3 blendFactors) {
    if(overrideEnabled) {
        return texture(ts, vec3(uv, overrideTexture));
    }
    
    vec4 res = vec4(vec3(0.0),1.0);
    for(int i=0 ; i < 3; ++i) {
        float weight = textureWeights[i];
        uint textureIndice = textureIndices[i];
        if(weight>0.0) {      
            res.rgb += texture(ts, vec3(uv, textureIndice)).rgb*weight;
        }
	}
    return res;
}

