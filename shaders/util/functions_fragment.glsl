
mat3 getTBN(vec3 pos, vec3 normal, vec2 uv, mat4 model, bool rotateTBN) {
    vec3 dpdx = dFdx(pos);
    vec3 dpdy = dFdy(pos);
    vec2 dudx = dFdx(uv);
    vec2 dudy = dFdy(uv);

    // Calculate the determinant for the tangent space matrix
    float det = dudx.x * dudy.y - dudx.y * dudy.x;
    float invDet = 1.0 / det;

    // Compute tangent and bitangent vectors
    vec3 tangent = normalize(invDet * (dpdx * dudy.y - dpdy * dudx.y));
    vec3 bitangent = normalize(invDet * (dpdy * dudx.x - dpdx * dudy.x));

    if(rotateTBN) {
        mat3 world = mat3(model); // Extract 3x3 from 4x4
        mat3 normal_matrix = transpose(inverse(world));
        normal = normalize(normal_matrix * normal);
        tangent = normalize(world * tangent);
        bitangent = normalize(world * bitangent);
    }

    // Transform normal map vector to world space
    return mat3(tangent, bitangent, normal);
}

vec2 parallaxMapping(in float ws[20], vec2 uv, vec3 viewDir, float scale, float minLayers, float maxLayers, int approxCycles) {
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
        currentHeight = textureBlend(ws, textures, currentUv, 2).r;

        if(currentHeight > currentDepth) {
            break;
        }
    }

    for (int i = 0; i < approxCycles; ++i) {
        vec2 midUv = 0.5 * (currentUv + prevUv);
        float midDepth = 0.5 * (currentDepth + prevDepth);
        float midHeight = textureBlend(ws, textures, midUv, 2).r;
        
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

ShadowProperties getShadow(sampler2D shadowMap, vec4 lightViewPosition, vec3 position) {
    vec3 shadowPosition = lightViewPosition.xyz / lightViewPosition.w; 
    float bias = 0.002;
    float shadow = texture(shadowMap, shadowPosition.xy).r < shadowPosition.z-bias ? 0.0 : 1.0;
    float texelSize = 1.0/4098.0;

    vec2 noiseCoords = (position.xy)* PI;
    float sumShadow = shadow;


    int blurRadius = 20;
    int totalSamples = 1;
    int maxSamples = 8;


    for(int radius = blurRadius; radius > 0; --radius) {
        for(int samples=0; samples < maxSamples ; ++samples) {

            vec4 noiseSample = texture(noise, noiseCoords);
            float sAngle = noiseSample.r * 2.0 * PI;
            float sRadius = radius;
            float sX = sRadius * cos(sAngle);
            float sY = sRadius * sin(sAngle);
            
            vec2 shadowUV = shadowPosition.xy+vec2(sX,sY)*texelSize;
            float shadowValue = texture(shadowMap, shadowUV).r;
            
            sumShadow += shadowValue < shadowPosition.z-bias ? 0.0 : 1.0;
            ++totalSamples;
            
            noiseCoords += noiseSample.xy;
        }
        if(sumShadow == totalSamples || sumShadow == 0) {
            break;
        }
    }

    ShadowProperties props;
    float shadowAlpha = 0.6;
    props.lightAmount = sumShadow/totalSamples;
    props.shadowAmount = (1.0 - shadowAlpha) + props.lightAmount*shadowAlpha;
    return props;
}