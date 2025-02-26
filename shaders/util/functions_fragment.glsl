
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


ShadowProperties getShadow(sampler2D shadowMap, sampler2D noise, vec4 lightViewPosition, vec3 position) {
    vec3 shadowPosition = lightViewPosition.xyz / lightViewPosition.w; 
    float bias = 0.002;
    float shadow = texture(shadowMap, shadowPosition.xy).r < shadowPosition.z-bias ? 0.0 : 1.0;
    float texelSize = 1.0/4098.0;

    vec2 noiseCoords = (position.xy)* PI;
    float sumShadow = shadow;


    int blurRadius = 20;
    int totalSamples = 1;
    int maxSamples = 6;
    int stepSize = 1;

    for(int radius = blurRadius; radius > 0; radius-=stepSize) {
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


vec4 textureBlend(vec3 ws, uvec3 ti, vec2 uv, int index) {
    if(overrideEnabled) {
        return texture(textures[overrideTexture], vec3(uv, index));
    }
    
    vec4 res = vec4(0.0);
    for(int i=0 ; i < 3; ++i) {
        float w = ws[i];
        uint t = ti[i];
        if(w>0.0) {      
            res += texture(textures[t], vec3(uv, index))*w;
        }
	}
    return res;
}

