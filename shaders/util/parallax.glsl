vec2 parallaxMapping(sampler2DArray ts, vec3 ws, uvec3 ti, vec2 uv, vec3 viewDir, float scale, float minLayers, float maxLayers, int approxCycles, vec3 blendFactors) {
    float numLayers = mix(maxLayers, minLayers, dot(vec3(0.0, 0.0, 1.0), -viewDir));  

	float deltaDepth = 1.0 / float( numLayers );
	vec2 deltaUv = (viewDir.xy/viewDir.z) * deltaDepth * scale;
	float currentDepth = 1.0;
    vec2 currentUv = uv;
    float currentHeight = 0.0;

    int cycles = 5;
    vec2 prevUv = currentUv;
    float prevDepth = currentDepth;
    float prevHeight = currentHeight;
    float bias = 0.1;
    
    for(int i=0; i < numLayers ; ++i) {
        prevUv = currentUv;
        prevHeight = currentHeight;
        prevDepth = currentDepth;

        currentUv -= deltaUv;
        currentDepth -= deltaDepth;
        currentHeight = textureBlend(ts, ws,ti, currentUv, blendFactors).r;

        if(currentHeight > currentDepth) {
            break;
        }
    }

    for (int i = 0; i < approxCycles; ++i) {
        vec2 midUv = 0.5 * (currentUv + prevUv);
        float midDepth = 0.5 * (currentDepth + prevDepth);
        float midHeight = textureBlend(ts, ws, ti, midUv,blendFactors).r;
        
        if (midHeight > midDepth) {
            currentUv = midUv;
            currentDepth = midDepth;
            currentHeight = midHeight;
        } else {
            prevUv = midUv;
            prevDepth = midDepth;
            prevHeight = midHeight;
        }
    }

    float delta1 = currentHeight - currentDepth; 
    float delta2 = ( currentDepth + deltaDepth ) - prevHeight; 
    float ratio = delta1/(delta1+delta2);
    return mix(currentUv, prevUv, ratio);
}