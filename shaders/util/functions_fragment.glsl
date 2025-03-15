


vec4 textureBlend(sampler2DArray ts, uvec3 textureIndices, vec2 uv, vec3 textureWeights, vec3 blendFactors) {
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