
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

