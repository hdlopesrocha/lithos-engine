
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


ShadowProperties getShadow(sampler2D shadowMap[SHADOW_MATRIX_COUNT], sampler2D noise, vec4 lightViewPosition[SHADOW_MATRIX_COUNT], vec3 position) {
    int selectedMap = 0;
    vec3 shadowPosition= lightViewPosition[selectedMap].xyz / lightViewPosition[selectedMap].w; 

    for(int i=0; i < SHADOW_MATRIX_COUNT ; ++i) {
        selectedMap = i;
        shadowPosition = lightViewPosition[i].xyz / lightViewPosition[i].w; 
        if(shadowPosition.x > 0.0 && shadowPosition.x < 1.0 && shadowPosition.y > 0.0 && shadowPosition.y < 1.0) {
            break;
        }
    }

    vec3 normal = normalize(cross(dFdx(position), dFdy(position))); // Approximate normal
    vec3 lightDir = normalize(shadowPosition - position);
    float bias = max(0.002 * (1.0 - dot(normal, lightDir)), 0.005);

    vec2 texelSize = 1.0/textureSize(shadowMap[selectedMap], 0);

    vec2 noiseCoords = (position.xy)* PI;
    float sumShadow = texture(shadowMap[selectedMap], shadowPosition.xy).r + bias > shadowPosition.z ? 1.0 : 0.0;
    int totalSamples = 1;



    int blurRadius = 16;
    int maxSamples = 6;
    int stepSize = 2;

    for(int radius = blurRadius; radius > 0; radius-=stepSize) {
        for(int samples=0; samples < maxSamples ; ++samples) {

            vec4 noiseSample = texture(noise, noiseCoords);
            float sAngle = noiseSample.r * 2.0 * PI;
            float sRadius = radius;
            float sX = sRadius * cos(sAngle);
            float sY = sRadius * sin(sAngle);
            
            vec2 shadowUV = clamp(shadowPosition.xy+vec2(sX,sY)*texelSize, 0.0, 1.0);
            sumShadow += texture(shadowMap[selectedMap], shadowUV).r + bias > shadowPosition.z ? 1.0 : 0.0;
            ++totalSamples;
            
            noiseCoords = noiseSample.xy;
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


vec4 textureBlend(sampler2DArray ts[25], vec3 ws, uvec3 ti, vec2 uv, int index) {
    if(overrideEnabled) {
        return texture(ts[overrideTexture], vec3(uv, index));
    }
    
    vec4 res = vec4(0.0);
    for(int i=0 ; i < 3; ++i) {
        float w = ws[i];
        uint t = ti[i];
        if(w>0.0) {      
            res += texture(ts[t], vec3(uv, index))*w;
        }
	}
    return res;
}

