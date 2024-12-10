#version 460 core

// Input from the vertex array
layout(location = 0) in vec3 position; 
layout(location = 1) in vec3 normal;    
layout(location = 2) in vec2 textureCoord;    
layout(location = 3) in uint textureIndex;    


// Output to the fragment shader
out float oTextureWeights[16];
out vec2 oTextureCoord;
out vec3 oNormal;

uniform mat4 model;      // Model transformation matrix
uniform mat4 view;       // View transformation matrix
uniform mat4 projection; // Projection transformation matrix
uniform vec3 lightDirection;     // Direction of the light (assumed to be normalized)

void main() {
    oNormal = mat3(transpose(inverse(model))) * normal;
      
    // Initialize texture weight to 0 for all other textures
    for (int i = 0; i < 16; ++i) {
        oTextureWeights[i] = 0.0;
    }

    // Set the weight for the current texture index
    oTextureWeights[textureIndex] = 1.0;
    
    oTextureCoord = textureCoord;


    // Calculate the final position of the vertex
    gl_Position = projection * view * model * vec4(position, 1.0);
}