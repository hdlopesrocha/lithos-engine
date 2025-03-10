float getShadow(sampler2D shadowMap[SHADOW_MATRIX_COUNT], sampler2D noise, vec4 lightViewPosition[SHADOW_MATRIX_COUNT], vec3 position, vec3 normal) {
    int selectedMap = 0;
    vec3 shadowPosition= lightViewPosition[selectedMap].xyz / lightViewPosition[selectedMap].w; 

    for(int i=0; i < SHADOW_MATRIX_COUNT ; ++i) {
        selectedMap = i;
        shadowPosition = lightViewPosition[i].xyz / lightViewPosition[i].w; 
        if(shadowPosition.x > 0.0 && shadowPosition.x < 1.0 && shadowPosition.y > 0.0 && shadowPosition.y < 1.0) {
            break;
        }
    }

    vec2 texelSize = 1.0/textureSize(shadowMap[selectedMap], 0);
    vec2 noiseCoords = (position.xy)* PI;
    float shadowDepth = texture(shadowMap[selectedMap], shadowPosition.xy).r; 
    vec3 lightDir = normalize(shadowPosition - position);


    // Angle-based bias (prevents self-shadowing at shallow angles)
    float angleBias = max(0.0005 * (1.0 - dot(normal, lightDir)), 0.001);

    // Blocker distance bias (avoids false shadows in corners)
    float blockerDistance = abs(texture(shadowMap[selectedMap], shadowPosition.xy).r - shadowPosition.z);
    float distanceBias = 0.0001 * log2(1.0 + blockerDistance); // Logarithmic scaling

    float bias = angleBias + distanceBias; // Combine both biases

    float blurFactor = smoothstep(0.0, 0.1, blockerDistance); 
    int blurRadius = int(clamp(mix(1.0, 128.0, blurFactor), 1.0, 16.0)); 


    int maxSamples = blurRadius;
    int stepSize = 1;
    float sumShadow = shadowDepth + bias > shadowPosition.z ? 1.0 : 0.0;
    int totalSamples = 1;


    for (int radius = blurRadius; radius > 0; radius -= stepSize) {
        for (int samples = 0; samples < maxSamples; ++samples) {
            
            vec4 noiseSample = texture(noise, noiseCoords);
            float randomAngle = noiseSample.r * 2.0 * PI;
            vec2 randomOffset = vec2(cos(randomAngle), sin(randomAngle)) * noiseSample.g; 
            noiseCoords += randomOffset * 0.1; // Small offset to vary noise sampling
         
            
            vec2 shadowUV = clamp(shadowPosition.xy + randomOffset*radius * texelSize, 0.0, 1.0);
            sumShadow += texture(shadowMap[selectedMap], shadowUV).r + bias > shadowPosition.z ? 1.0 : 0.0;
            ++totalSamples;
        }
        if (sumShadow == totalSamples || sumShadow == 0) {
            break;
        }
    }
    return sumShadow/totalSamples;
}
