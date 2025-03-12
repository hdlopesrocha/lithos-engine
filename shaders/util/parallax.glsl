

vec2 parallaxMapping(
        sampler2DArray ts, 
        uvec3 textureIndices, 
        vec2 uv, 
        vec3 viewDir, 
        TextureBrush props, 
        vec3 textureWeights, 
        vec3 blendFactors, 
        float distanceFactor
    ) {
    
    
    float scale= props.parallaxScale * distanceFactor;
    float minLayers = props.parallaxMinLayers * distanceFactor;
    float maxLayers = props.parallaxMaxLayers * distanceFactor;
    int approxCycles = int(ceil(distanceFactor * props.parallaxRefine));
    if(approxCycles <=0){
        approxCycles = 1;
    }
        

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
        currentHeight = textureBlend(ts,textureIndices, currentUv, textureWeights, blendFactors).r;

        if(currentHeight > currentDepth) {
            break;
        }
    }

    for (int i = 0; i < approxCycles; ++i) {
        vec2 midUv = 0.5 * (currentUv + prevUv);
        float midDepth = 0.5 * (currentDepth + prevDepth);
        float midHeight = textureBlend(ts, textureIndices, midUv,textureWeights,blendFactors).r;
        
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