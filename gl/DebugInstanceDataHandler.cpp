#include "gl.hpp"


glm::vec3 DebugInstanceDataHandler::getCenter(DebugInstanceData instance) {
    return glm::vec3(instance.matrix[3]);
};

void DebugInstanceDataHandler::bindInstance(GLuint instanceBuffer, std::vector<DebugInstanceData> * instances) {
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, instances->size() * sizeof(DebugInstanceData), instances->data(), GL_STATIC_DRAW);

    for (int i = 0; i < 8; i++) {
        glVertexAttribPointer(4 + i, 1, GL_FLOAT, GL_FALSE, sizeof(DebugInstanceData), 
            (void*)(offsetof(DebugInstanceData, sdf) + i * sizeof(float)));
        glEnableVertexAttribArray(4 + i);
        glVertexAttribDivisor(4 + i, 1); // Enable instancing
    }

    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(12 + i, 4, GL_FLOAT, GL_FALSE, sizeof(DebugInstanceData), 
            (void*)(offsetof(DebugInstanceData, matrix) + i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(12 + i);
        glVertexAttribDivisor(12 + i, 1); // Enable instancing
    }

}

