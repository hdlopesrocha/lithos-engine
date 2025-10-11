#include "gl.hpp"


glm::vec3 DebugInstanceDataHandler::getCenter(DebugInstanceData instance) {
    return glm::vec3(instance.matrix[3]);
};

void DebugInstanceDataHandler::bindInstance(GLuint instanceBuffer, std::vector<DebugInstanceData> * instances) {
    glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, instances->size() * sizeof(DebugInstanceData), instances->data(), GL_STATIC_DRAW);

    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(4 + i, 4, GL_FLOAT, GL_FALSE, sizeof(DebugInstanceData), 
            (void*)(offsetof(DebugInstanceData, matrix) + i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(4 + i);
        glVertexAttribDivisor(4 + i, 1); // Enable instancing
    }

    // sdf1
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(DebugInstanceData),
        (void*)offsetof(DebugInstanceData, sdf1));
    glEnableVertexAttribArray(8);
    glVertexAttribDivisor(8, 1);

    // sdf2
    glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, sizeof(DebugInstanceData),
        (void*)offsetof(DebugInstanceData, sdf2));
    glEnableVertexAttribArray(9);
    glVertexAttribDivisor(9, 1);

    glVertexAttribIPointer(10, 1, GL_INT, sizeof(DebugInstanceData),
        (void*)offsetof(DebugInstanceData, brushIndex));
    glEnableVertexAttribArray(10);
    glVertexAttribDivisor(10, 1);

}

