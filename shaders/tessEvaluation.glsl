#version 460 core

layout(triangles, equal_spacing, ccw) in; // Define primitive type and tessellation spacing



in float tcTextureWeights[][16];
in vec2 tcTextureCoord[];
in vec3 tcNormal[];
in vec3 tcPosition[];


out vec3 teNormal;
out vec2 teTextureCoord;
out float teTextureWeights[16];
out vec3 tePosition;



uniform mat4 model;      // Model transformation matrix
uniform mat4 view;       // View transformation matrix
uniform mat4 projection; // Projection transformation matrix


void main() {
    // Barycentric coordinates for the current vertex
    vec3 barycentric = gl_TessCoord; // (u, v, w) barycentric coordinates


   // Interpolate attributes using barycentric coordinates
    teNormal = tcNormal[0] * gl_TessCoord.x + tcNormal[1] * gl_TessCoord.y + tcNormal[2] * gl_TessCoord.z;

    teNormal = normalize(mat3(transpose(inverse(model))) * teNormal);

    teTextureCoord = tcTextureCoord[0] * gl_TessCoord.x + tcTextureCoord[1] * gl_TessCoord.y + tcTextureCoord[2] * gl_TessCoord.z;
    
    for (int i = 0; i < 16; ++i) {
        teTextureWeights[i] = (barycentric.x * tcTextureWeights[0][i] + barycentric.y * tcTextureWeights[1][i] + barycentric.z * tcTextureWeights[2][i]);
    }


    // Interpolate the triangle position using barycentric coordinates
    vec3 pos = barycentric.x * tcPosition[0] +
                  barycentric.y * tcPosition[1] +
                  barycentric.z * tcPosition[2];

    tePosition = pos;
    gl_Position = projection * view * model * vec4(pos, 1.0);     
}